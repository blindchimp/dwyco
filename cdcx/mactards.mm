
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dlli.h"
#include <string>
#include <sstream>
#include <Foundation/Foundation.h>

static
void
mac_log_error(const char *)
{
    ::abort();
}

static
std::string GetAppCacheDirectory(const std::string appName)
{
// NSApplicationSupportDirectory
    std::string appCacheDir = "";

    NSArray * dirs = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, TRUE);
    if (nil == dirs || [dirs count] != 1)
    {
        ::abort();
    }
    else
    {
        NSString * supportDir = [dirs objectAtIndex:0];
        NSString * cacheDir = [supportDir stringByAppendingPathComponent:@"CDC-X"];
        appCacheDir = [cacheDir UTF8String];
    }
    return appCacheDir;
}


static
std::string GetAppRunDirectory(std::string appName)
{
    std::string a;
    a = GetAppCacheDirectory(appName);
    a += "/" ;
    a += appName ;
    a += "-run";
    return a;
}

static
void SetCurrentDirectory(const std::string & dir)
{
    NSString *curDir = [[NSFileManager defaultManager] currentDirectoryPath];
    NSString *targetDir = [NSString stringWithCString:dir.c_str() encoding:NSASCIIStringEncoding];

    if (![curDir isEqualToString:targetDir])
    {
        [[NSFileManager defaultManager] changeCurrentDirectoryPath:targetDir];
    }
}

/* static */
static
std::string GetUserHomeDirectory()
{
    return std::string([NSHomeDirectory() UTF8String]);
}

/* static */
static
bool MkDir(const std::string & path, bool makeIntermediates)
{
    NSFileManager * fileMgr = [NSFileManager defaultManager];
    NSString * nsPath = [NSString stringWithUTF8String:path.c_str()];
    NSError * err = nil;
    [fileMgr createDirectoryAtPath:nsPath withIntermediateDirectories:makeIntermediates attributes:nil error:&err];
    return (err == nil || [err code] == noErr);

}
/* static */
static
bool FileExists(const std::string & path)
{
    NSFileManager * fileMgr = [NSFileManager defaultManager];
    NSString * nsPath = [NSString stringWithUTF8String:path.c_str()];
    return (bool) [fileMgr fileExistsAtPath:nsPath];
}

static
bool DirExists(const std::string & path)
{
    return FileExists(path);
}


static
void install_run(const std::string & targetPath)
{
    // create run dir including intermediate dirs
    if (!MkDir(targetPath, true))
    {
        mac_log_error("FATAL: could not create CDC-X-run directory");
        exit(0);
    }

    // locate CDC-X-install.zip file
    NSString * zipSource = [[NSBundle mainBundle] pathForResource:@"CDC-X-install" ofType:@"zip"];
    if (![[NSFileManager defaultManager] fileExistsAtPath:zipSource])
    {
        mac_log_error("FATAL: could not locate CDC-X-install.zip resource in bundle; application may be corrupt.");
        exit(0);
    }

    std::string ssUnzipCommand;
    ssUnzipCommand = "unzip -o \"" ;
    ssUnzipCommand += [zipSource UTF8String] ;
    ssUnzipCommand += "\" -d \"" ;
    ssUnzipCommand += targetPath ;
    ssUnzipCommand += "\"";
    NSLog(@"running command:\n%s", ssUnzipCommand.c_str());
    std::system(ssUnzipCommand.c_str());

    // Verify that it worked by checking for one of the known files - CDC-X-install/license.txt
    NSString * licenseFilePath = [[NSString stringWithUTF8String:targetPath.c_str()] stringByAppendingPathComponent:@"license.txt"];
    if (![[NSFileManager defaultManager] fileExistsAtPath:licenseFilePath])
    {
        mac_log_error("DLL ARCHIVE MAY BE CORRUPT: did not detect dwyco license.txt in run directory after unzipping.");
    }
}

void EstablishRunDirectory()
{
    std::string cdc_x_RunDir = GetAppRunDirectory("CDC-X");
    if (!DirExists(cdc_x_RunDir) ||
            !FileExists(cdc_x_RunDir + "/v21.ver") ||
            !FileExists(cdc_x_RunDir + "/v29.ver") ||
            !FileExists(cdc_x_RunDir + "/v211.ver") ||
            !FileExists(cdc_x_RunDir + "/v216.ver") ||
            !FileExists(cdc_x_RunDir + "/v230.ver") ||
            !FileExists(cdc_x_RunDir + "/v232.ver") ||
            !FileExists(cdc_x_RunDir + "/v217.ver")
       )
    {
        // Need to unzip the run-directory resource to the destination
        install_run(cdc_x_RunDir);
    }

    // Make sure cwd of app is run-dir
    SetCurrentDirectory(cdc_x_RunDir);

    // Tell dll about its new home
    //dwyco_set_cmd_path(cdc_x_RunDir.c_str(), cdc_x_RunDir.length());

}
