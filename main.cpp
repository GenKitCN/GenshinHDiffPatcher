#include "filesystem"
#include "iostream"
#include "fstream"
#include "vector"
#include "cxxopts.hpp"
#include "HPatch/patch.h"
#include "file_for_patch.h"
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"
#include "elzip/elzip.hpp"

#define check_on_error(errorType) { \
    if ( patchResult == 0) patchResult = errorType; if (!_isInClear){ goto clear; } }
#define check(value, errorType, errorInfo) { \
    if (!(value)){ spdlog::error("{} ERROR!", errorInfo); check_on_error(errorType); } }
# define check_ferr(fileError, errorType, errorInfo){ \
    if (fileError){ \
            if ( ENOSPC==(fileError)){ \
                check(hpatch_FALSE, 24, errorInfo); \
            } else{ check(hpatch_FALSE, errorType, errorInfo); } } }
#define _free_mem(p) { if (p) { free(p); p=0; } }
#define kPatchCacheSize_min      (hpatch_kStreamCacheSize*8)
#define kPatchCacheSize_bestmin  ((size_t)1<<21)
#define _kNULL_SIZE     (~(size_t)0)

namespace fs = std::filesystem;
fs::path PROGRAM_DIR = fs::current_path();


static TByte *getPatchMemCache(size_t mustAppendMemSize, hpatch_StreamPos_t oldDataSize, size_t *out_memCacheSize) {
    TByte *temp_cache = nullptr;
    size_t temp_cache_size;
    size_t patchCacheSize = 1 << 22;
    {
        hpatch_StreamPos_t limitCacheSize;
        const hpatch_StreamPos_t bestMaxCacheSize = oldDataSize + kPatchCacheSize_bestmin;
        limitCacheSize = patchCacheSize;
        limitCacheSize = (limitCacheSize < bestMaxCacheSize) ? limitCacheSize : bestMaxCacheSize;
        if (limitCacheSize > (size_t) (_kNULL_SIZE - mustAppendMemSize))//too large
            limitCacheSize = (size_t) (_kNULL_SIZE - mustAppendMemSize);
        temp_cache_size = (size_t) limitCacheSize;
    }
    while (temp_cache_size >= kPatchCacheSize_min) {
        temp_cache = (TByte *) malloc(mustAppendMemSize + temp_cache_size);
        if (temp_cache) break;
        temp_cache_size >>= 1;
    }
    *out_memCacheSize = (temp_cache) ? (mustAppendMemSize + temp_cache_size) : 0;
    return temp_cache;
}


int hpatch(const std::filesystem::path &fromPth, const fs::path &diffPth, const fs::path &toPth) {
    spdlog::info("Patching {}.", toPth.filename().string());
    const char *oldFilePathChar, *diffFilePathChar, *newFilePathChar;
    std::string oldFilePath = fromPth.string(),
            diffFilePath = diffPth.string(),
            newFilePath = toPth.string();

    int patchResult = 0;
    oldFilePathChar = oldFilePath.c_str();
    diffFilePathChar = diffFilePath.c_str();
    newFilePathChar = newFilePath.c_str();
    int _isInClear = hpatch_FALSE;
    hpatch_TDecompress _decompressPlugin {nullptr};
    hpatch_TDecompress *decompressPlugin = &_decompressPlugin;
    hpatch_TFileStreamInput oldFileStream;
    hpatch_TFileStreamInput diffFileStream;
    hpatch_TFileStreamOutput newFileStream;
    hpatch_TStreamInput *poldData = &oldFileStream.base;
    TByte *temp_cache = nullptr;
    size_t temp_cache_size;
    hpatch_StreamPos_t savedNewSize = 0;
    int patch_result = 0;

    // init
    hpatch_TFileStreamInput_init(&oldFileStream);
    hpatch_TFileStreamInput_init(&diffFileStream);
    hpatch_TFileStreamOutput_init(&newFileStream);

    check(hpatch_TFileStreamInput_open(&oldFileStream, oldFilePathChar), 7,
          "opening old: " + oldFilePath)
    check(hpatch_TFileStreamInput_open(&diffFileStream, diffFilePathChar), 7,
          "opening diff: " + diffFilePath)

    {
        hpatch_compressedDiffInfo diffInfo;
        if (!getCompressedDiffInfo(&diffInfo, &diffFileStream.base)) {
            check(!diffFileStream.fileError, 4, "read diffFile")
            check(hpatch_FALSE, 9, "is hdiff file? get diffInfo")
        }
        if (poldData->streamSize != diffInfo.oldDataSize) {
            spdlog::error("oldFile dataSize {:d} != diffFile saved oldDataSize {:d} ERROR!\n",
                    poldData->streamSize, diffInfo.oldDataSize);
            check_on_error(6)
        }
        if (decompressPlugin->open == nullptr) decompressPlugin = nullptr;
        savedNewSize = diffInfo.newDataSize;
    }
    check(hpatch_TFileStreamOutput_open(&newFileStream, newFilePathChar, savedNewSize),
          3, "open out newFile for write")
    spdlog::info("HDiff | oldDataSize: {:d}", poldData->streamSize);
    spdlog::info("HDiff | diffDataSize: {:d}", diffFileStream.base.streamSize);
    spdlog::info("HDiff | newDataSize: {:d}", newFileStream.base.streamSize);

    {
        hpatch_StreamPos_t maxWindowSize = poldData->streamSize;
        hpatch_size_t mustAppendMemSize = 0;
        temp_cache = getPatchMemCache(mustAppendMemSize, maxWindowSize, &temp_cache_size);
    }
    check(temp_cache, 8, "alloc cache memory")
    {
        if (!patch_decompress_with_cache(&newFileStream.base, poldData, &diffFileStream.base, decompressPlugin,
                                         temp_cache, temp_cache + temp_cache_size))
            patch_result = 11;
    }
    if (patch_result != 0) {
        check(!oldFileStream.fileError, 4, "oldFile read")
        check(!diffFileStream.fileError, 4, "diffFile read")
        check_ferr(newFileStream.fileError, 5, "out newFile write")
        check(hpatch_FALSE, patch_result, "patch run")
    }
    if (newFileStream.out_length != newFileStream.base.streamSize) {
        spdlog::error("out newFile dataSize {:d} != diffFile saved newDataSize {:d} ERROR!",
                newFileStream.out_length, newFileStream.base.streamSize);
        check_on_error(6)
    }
    spdlog::info("HDiff | patch ok!");

    clear:
    _isInClear = hpatch_TRUE;
    check(hpatch_TFileStreamOutput_close(&newFileStream), 7, "close" + newFilePath)
    check(hpatch_TFileStreamInput_close(&diffFileStream), 7, "close" + diffFilePath)
    check(hpatch_TFileStreamInput_close(&oldFileStream), 7, "close" + oldFilePath)
    _free_mem(temp_cache)

    return patchResult;
}


bool askFor(const std::string& question, bool defaultValue = true)
{
    std::cout << question << " [" << (defaultValue ? "Yn" : "Ny") << "] ";
    std::string response;
    std::getline(std::cin, response);
    if (response.empty()) {
        return defaultValue;
    }
    if (response[0] == 'y' || response[0] == 'Y') {
        return true;
    } else if (response[0] == 'n' || response[0] == 'N') {
        return false;
    } else {
        std::cerr << "Invalid response. Please enter 'y' or 'n'." << std::endl;
        return askFor(question, defaultValue);
    }
}


int main(int argc, const char *argv[]) {

    cxxopts::Options parser("GenshinPatcher", "Automatically update certain anime game's hdiff package command line");
    parser.add_options()
            ("gameDir", "Path of GenshinImpact", cxxopts::value<std::string>())
            ("diffFiles", "HDiff package", cxxopts::value<std::vector<std::string>>())
            ("s,safe-patch", "Run the patch safely.", cxxopts::value<bool>())
            ("f,force-del",
             "Forces the file to be deleted according to  deletefiles.txt, even though --safe-patch is specified.")
            ("h,help", "Show this msg");
    parser.positional_help("<gameDir> <diffFiles> [diffFiles2] [diffFiles3]...");
    parser.parse_positional({"gameDir", "diffFiles"});

    fs::path gameRoot, patchRoot;
    std::vector<std::string> diffFilesString;
    try {
        auto args = parser.parse(argc, argv);
        gameRoot = fs::path(args["gameDir"].as<std::string>());
        if (args["safe-patch"].as<bool>()) {
            patchRoot = gameRoot / "temp";
        } else patchRoot = gameRoot;
        diffFilesString = args["diffFiles"].as<std::vector<std::string>>();
        if (args.count("help")) {
            std::cout << parser.help() << std::endl;
            return 0;
        }
    } catch ( cxxopts::exceptions::option_has_no_value &e) {
        spdlog::error(e.what());
        std::cout << parser.help();
        return 1;
    }

    std::vector<fs::path> diffFiles;
    std::transform(diffFilesString.begin(), diffFilesString.end(), std::back_inserter(diffFiles),
                   [](const std::string &str) { return std::filesystem::path(str); });

    fs::path tmp_dir = PROGRAM_DIR / "tmp";

    try {
        // 解压zip文件
        for (auto &s: diffFiles) {
            fs::path diffFolder, extractDst;
            if (!fs::is_directory(s)) {
                if (!fs::exists(tmp_dir)) {
                    spdlog::info("Creating folder \"tmp\" to store the extracted files.");
                }
                if (s.string().substr(s.string().find_last_of('.') + 1) != "zip") continue;
                extractDst = tmp_dir / s.filename().string().substr(0, s.filename().string().length() - 4);
                spdlog::info("Extracting {}...", s.filename().string());
                elz::extractZip(s, extractDst);
                spdlog::info(" -> {}", extractDst.string());
                diffFolder = extractDst;
            } else {
                diffFolder = s;
            }

            // diff和delete列表
            std::vector<fs::path> hdiffFilesList;
            std::vector<fs::path> deleteFileList;

            if ( fs::exists(diffFolder / "hdifffiles.txt") ) {
                std::ifstream hdiffFileStream(diffFolder / "hdifffiles.txt");
                for (std::string line; getline(hdiffFileStream, line);) {
                    std::string hdiffFileString = nlohmann::json::parse(line)["remoteName"].get<std::string>() + ".hdiff";
                    fs::path hdiffFilePth = diffFolder / hdiffFileString;
                    hdiffFilesList.push_back(hdiffFilePth);
                }
                hdiffFileStream.close();
            }
            if ( fs::exists(diffFolder / "deletefiles.txt") ) {
                std::ifstream deleteFileStream(diffFolder / "deletefiles.txt");
                for (std::string line; getline(deleteFileStream, line);) {
                    deleteFileList.emplace_back(gameRoot / line);
                }
                deleteFileStream.close();
            }

            // 处理不在hdiffFile里的文件
            for (const auto &entry: fs::recursive_directory_iterator(diffFolder)) {
                if (entry.path().filename() == "hdifffiles.txt" ||
                    entry.path().filename() == "deletefiles.txt" ||
                    std::binary_search(hdiffFilesList.begin(), hdiffFilesList.end(), entry.path())
                    ) continue;

                fs::path dstFilePath = patchRoot / fs::relative(entry, diffFolder);
                if (!fs::exists(dstFilePath)) {
                    if (entry.is_directory()) {
                        fs::copy(entry, dstFilePath, fs::copy_options::update_existing);
                    } else {
                        fs::create_directories(dstFilePath.parent_path());
                        fs::copy_file(entry, dstFilePath, fs::copy_options::update_existing);
                    }
                }
            }

            // 处理位于hdiffFile的文件
            if (!exists(patchRoot)) fs::create_directories(patchRoot);
            for (const auto &h: hdiffFilesList) {
                try {
                    fs::path relative2root = fs::relative(h, diffFolder);
                    fs::path rawFilePath = relative2root.string().substr(0, relative2root.string().length() - 6);
                    hpatch(gameRoot / rawFilePath, h, patchRoot / rawFilePath);
                } catch (const std::exception &e) {
                    spdlog::error(e.what());
                }
            }

            if ( patchRoot != gameRoot ) { // safe-patch enabled
                if ( !askFor("Do you want CLI to help you move the files?") ) goto do_del;
                for ( const auto &di : fs::directory_iterator(patchRoot)) {
                    fs::path copyDst = gameRoot / fs::relative(di, patchRoot);
                    fs::copy_file(di, copyDst, fs::copy_options::recursive);
                    if ( fs::exists(copyDst) ) fs::remove(di);
                }
            }

            do_del: // 处理deletefiles
            if ( deleteFileList.empty() ||
                 !askFor("Do you want CLI to help you delete the files from \"deletefiles.txt\"?") ) goto del_extract_folder;
            for (const auto &d: deleteFileList) {
                try {
                    fs::path relative2root = fs::relative(d, gameRoot);
                    spdlog::info("Deleting {}.", relative2root.string());
                    fs::remove(d);
                } catch (fs::filesystem_error::exception &e) {
                    spdlog::error(e.what());
                }
            }
            // 删除临时解压目录
            del_extract_folder:
            if (fs::exists(extractDst)) {
                spdlog::info("CClean up the temporary extracted folder: {}", extractDst.string());
                fs::remove_all(extractDst);
            }
        }
    } catch (const std::exception &e) {
        spdlog::warn(e.what());
    }

    return 0;
}
