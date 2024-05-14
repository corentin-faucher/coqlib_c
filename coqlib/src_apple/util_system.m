//
//  util_system.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-25.
//

#include "util_system.h"

#import <Foundation/Foundation.h>
#import <CloudKit/CloudKit.h>

#include "util_base.h"
#include "util_string.h"
#include "coq_event.h"

#if TARGET_OS_OSX == 1
#import <Carbon/Carbon.h>
#import <AppKit/AppKit.h>
#else
#import <UIKit/UIKit.h>
#endif

static char* coqsystem_layoutName_ = NULL;
static unsigned coqsystem_keyboardtype_ = keyboardtype_ansi;
static unsigned coqsystem_os_type_ = coqsystem_os_desktop;
static char* coqsystem_os_version_ = NULL;
static char* coqsystem_app_version_ = NULL;
static char* coqsystem_app_build_ = NULL;
static char* coqsystem_app_name_ = NULL;

void  CoqSystem_setOs_(void) {
#if TARGET_OS_OSX == 1
    coqsystem_os_type_ = coqsystem_os_desktop;
#else
    // (On suppose que si c'est pas un téléphone, c'est une tablette...)
    if([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        coqsystem_os_type_ = coqsystem_os_phone;
    } else {
        coqsystem_os_type_ = ([[UIScreen mainScreen] nativeBounds].size.height < 2400) ? coqsystem_os_tablet : coqsystem_os_tablet_big;
    }
#endif
}
void  CoqSystem_cloudDrive_init_(void); // (voir plus bas)
void  CoqSystem_init(void) {
    if(coqsystem_os_version_ != NULL) { printerror("Already init."); return; }
    // Os version
    CoqSystem_setOs_();
#if TARGET_OS_OSX == 1
    NSString* os_version = [NSString stringWithFormat:@"macOS %@", [[NSProcessInfo processInfo] operatingSystemVersionString]];
#else
    NSString* os_version = [NSString stringWithFormat:@"iOS %@",   [[NSProcessInfo processInfo] operatingSystemVersionString]];
#endif
    coqsystem_os_version_ = String_createCopy([os_version UTF8String]);
    // App version
    NSString* app_version = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    coqsystem_app_version_ = String_createCopy([app_version UTF8String]);
    // App build
    NSString* app_build = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"];
    coqsystem_app_build_ = String_createCopy([app_build UTF8String]);
    // App name
    NSString* app_name = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
    coqsystem_app_name_ = String_createCopy([app_name UTF8String]);
#if TARGET_OS_OSX == 1
    // Layout name
    CoqSystem_layoutUpdate();
    // Keyboard type
    switch(KBGetLayoutType(LMGetKbdType())) {
        case kKeyboardANSI: coqsystem_keyboardtype_ = keyboardtype_ansi; break;
        case kKeyboardISO:  coqsystem_keyboardtype_ = keyboardtype_iso; break;
        case kKeyboardJIS:  coqsystem_keyboardtype_ = keyboardtype_jis; break;
    }
#endif
    // Theme
    CoqSystem_theme_OsThemeUpdate();
    // iCloud
    CoqSystem_cloudDrive_init_();
}

#pragma mark - Keyboard Layout

const char* CoqSystem_layoutOpt(void) {
    return coqsystem_layoutName_;
}
void        CoqSystem_layoutUpdate(void) {
#if TARGET_OS_OSX == 1
    if(coqsystem_layoutName_) {
        coq_free(coqsystem_layoutName_);
        coqsystem_layoutName_ = NULL;
    }
    TISInputSourceRef inputSrc = TISCopyCurrentKeyboardInputSource();
    if(inputSrc == nil) {
        printwarning("Keyboard input source not found.");
        return;
    }
    CFStringRef property = (CFStringRef)TISGetInputSourceProperty(inputSrc, kTISPropertyInputSourceID);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(
                CFStringGetLength(property), kCFStringEncodingUTF8) + 1;
    coqsystem_layoutName_ = coq_calloc(maxSize, sizeof(char));
    CFStringGetCString(property, coqsystem_layoutName_, maxSize, kCFStringEncodingUTF8);
#else
    printwarning("Cannot detect keyboard layout in iOS.");
#endif
}
unsigned    CoqSystem_keyboardType(void) {
#if TARGET_OS_OSX != 1
    printwarning("CoqSystem_keyboardType is only valid in macOS.");
#endif
    return coqsystem_keyboardtype_;
}

#pragma mark - App version

unsigned    CoqSystem_OS_type(void) {
    return coqsystem_os_type_;
}
void        CoqSystem_OS_forceTo(unsigned coqsystem_os) {
    if(coqsystem_os >= coqsystem_os__default_) {
        CoqSystem_setOs_();
    } else {
        coqsystem_os_type_ = coqsystem_os;
    }
}

const char*  CoqSystem_OS_versionOpt(void) {
    if(coqsystem_os_version_ == NULL) { printerror("System not init."); return NULL; }
    return coqsystem_os_version_;
}
const char*  CoqSystem_appVersionOpt(void) {
    if(coqsystem_app_version_ == NULL) { printerror("System not init."); return NULL; }
    return coqsystem_app_version_;
}
const char*  CoqSystem_appBuildOpt(void) {
    if(coqsystem_app_build_ == NULL) { printerror("System not init."); return NULL; }
    return coqsystem_app_build_;
}
const char* CoqSystem_appDisplayNameOpt(void) {
    if(coqsystem_app_name_ == NULL) { printerror("System not init."); return NULL; }
    return coqsystem_app_name_;
}

#pragma mark - Theme (Dark, light)

static bool os_theme_is_dark_ = false;
static bool current_theme_is_dark_ = false;
static bool current_theme_as_os_theme_ = true;
void        CoqSystem_theme_OsThemeUpdate(void) {
#if TARGET_OS_OSX == 1
    NSAppearance* appearance;
    if (@available(macOS 11.0, *)) {
        appearance = NSAppearance.currentDrawingAppearance;
    } else {
        appearance = NSAppearance.currentAppearance;
    }
    os_theme_is_dark_ = [@[
        NSAppearanceNameDarkAqua, NSAppearanceNameVibrantDark,
        NSAppearanceNameAccessibilityHighContrastDarkAqua,
        NSAppearanceNameAccessibilityHighContrastVibrantDark
    ] containsObject:[appearance name]];
#else
    if(@available(iOS 13.0, *)) {
        os_theme_is_dark_ = [[UITraitCollection currentTraitCollection]
                             userInterfaceStyle] == UIUserInterfaceStyleDark;
    } else {
        os_theme_is_dark_ = false;
    }
#endif
    if(current_theme_as_os_theme_) {
        current_theme_is_dark_ = os_theme_is_dark_;
    }
}
bool        CoqSystem_theme_OsThemeIsDark(void) {
    return os_theme_is_dark_;
}
void        CoqSystem_theme_setAppTheme(bool isDark) {
    current_theme_is_dark_ = isDark;
    current_theme_as_os_theme_ = false;
}
void        CoqSystem_theme_setAppThemeToOsTheme(void) {
    current_theme_is_dark_ = os_theme_is_dark_;
    current_theme_as_os_theme_ = true;
}
bool        CoqSystem_theme_appThemeIsDark(void) {
    return current_theme_is_dark_;
}

#pragma mark - iCloud Drive

// TODO... pour les "Documents", i.e. visible dans iCloud Drive.
@interface ICloudDriveManager : NSObject {
    NSMetadataQuery* metaDataQuery;
@public
    BOOL             updating;
    BOOL             iCloudEnabled;
}
@end
@implementation ICloudDriveManager
-(void)iCloudChanged {
    iCloudEnabled = [[NSFileManager defaultManager] ubiquityIdentityToken] != nil;
    CoqEvent_addToRootEvent((CoqEvent) {
        .type = event_type_systemChanged,
        .system_change = { .cloudDriveDidChange = true },
    });
}
-(id)init {
    self = [super init];
    iCloudEnabled = [[NSFileManager defaultManager] ubiquityIdentityToken] != nil;
    updating = false;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(iCloudChanged)
        name:NSUbiquityIdentityDidChangeNotification object:nil];
    
    return self;
}
-(void)dataQueryDidUpdate {
    __block bool newUpdating = false;
    if(metaDataQuery == nil) {
        printerror("No data query.");
    } else { [metaDataQuery enumerateResultsUsingBlock:^(id  _Nonnull item, NSUInteger index, BOOL * _Nonnull stop) {
        NSMetadataItem* metaDataItem = (NSMetadataItem*)item;
//        printdebug("checking metadata %s, path key %s.",
//            [[metaDataItem valueForAttribute:NSMetadataItemFSNameKey] UTF8String],
//            [[metaDataItem valueForAttribute:NSMetadataItemPathKey] UTF8String]
//        );
        NSURL* url = [metaDataItem valueForAttribute:NSMetadataItemURLKey];
        NSString* downloadStatus = [metaDataItem valueForAttribute:NSMetadataUbiquitousItemDownloadingStatusKey];
        if([downloadStatus isEqual:NSMetadataUbiquitousItemDownloadingStatusCurrent]) {
            // Cas "OK", fini d'updater.
//            printdebug("✅: %s.", [[url relativeString] UTF8String]);
        } else if ([downloadStatus isEqual:NSMetadataUbiquitousItemDownloadingStatusDownloaded]) {
            // Cas "de trop" sera effacé...
            newUpdating = true;
        } else if ([downloadStatus isEqual:NSMetadataUbiquitousItemDownloadingStatusNotDownloaded]) {
            // Cas "manquant"
            newUpdating = true;
            // Demande de téléchargement...
            NSError* error;
            [[NSFileManager defaultManager] startDownloadingUbiquitousItemAtURL:url error:&error];
            if(error != nil) {
                printf("Error %s", [[error localizedDescription] UTF8String]);
            }
        } else {
            printerror("Undefined data query status.");
        }
    }];}
    // (pas de changement)
    if(newUpdating == updating)
        return;
    updating = newUpdating;
    CoqEvent_addToRootEvent((CoqEvent) {
        .type = event_type_systemChanged,
        .system_change = { .cloudDriveDidChange = true },
    });
}
-(void)startDataQueryIn:(nullable NSString*)folderOpt withExtension:(nullable NSString*) extension {
    if(metaDataQuery) return;
    updating = true;
    metaDataQuery = [[NSMetadataQuery alloc] init];
    [metaDataQuery setSearchScopes:@[NSMetadataQueryUbiquitousDataScope]];
    if(folderOpt || extension) {
        NSString* format;
        if(folderOpt) {
            NSURL* cloudUrl = [[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:nil];
            cloudUrl = [cloudUrl URLByAppendingPathComponent:folderOpt isDirectory:YES];
            if(extension)
                format = [NSString stringWithFormat:@"%@ BEGINSWITH \"%@\" AND %@ LIKE \"*.%@\"",
                            NSMetadataItemPathKey, cloudUrl.path, NSMetadataItemFSNameKey, extension];
            else
                format = [NSString stringWithFormat:@"%@ BEGINSWITH \"%@\"",
                            NSMetadataItemPathKey, cloudUrl.path];
        } else {
            format = [NSString stringWithFormat:@"%@ LIKE \"*.%@\"",
                            NSMetadataItemFSNameKey, extension];
        }
        NSPredicate* predicate = [NSPredicate predicateWithFormat:format];
        [metaDataQuery setPredicate:predicate];
    }
    [metaDataQuery startQuery];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(dataQueryDidUpdate)
        name:NSMetadataQueryDidUpdateNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(dataQueryDidUpdate)
        name:NSMetadataQueryDidFinishGatheringNotification object:nil];
}
-(void)stopDataQuery {
    if(!metaDataQuery) return;
    [[NSNotificationCenter defaultCenter] removeObserver:self
        name:NSMetadataQueryDidUpdateNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self
        name:NSMetadataQueryDidFinishGatheringNotification object:nil];
    [metaDataQuery stopQuery];
    metaDataQuery = nil;
    updating = false;
}
@end

ICloudDriveManager* cloudDrive_manager_ = nil;
void        CoqSystem_cloudDrive_init_(void) {
    if(cloudDrive_manager_ != nil) {
        printwarning("Cloud drive manager already set.");
        return;
    }
    cloudDrive_manager_ = [[ICloudDriveManager alloc] init];
}
void        CoqSystem_cloudDrive_startWatching_(const char* subFolderOpt, const char* extensionOpt) {
    if(cloudDrive_manager_ == nil) {
        printerror("No cloud drive manager.");
        return;
    }
    if(cloudDrive_manager_->iCloudEnabled) {
        NSString* folder = nil;
        NSString* extension = nil;
        if(subFolderOpt) folder = [NSString stringWithUTF8String:subFolderOpt];
        if(extensionOpt) extension = [NSString stringWithUTF8String:extensionOpt];
        [cloudDrive_manager_ startDataQueryIn:folder withExtension:extension];
    } else {
        printwarning("iCloud not enabled.");
    }
    
}
bool        CoqSystem_cloudDrive_isEnabled(void) {
    if(cloudDrive_manager_ == nil) {
        printwarning("No cloud drive manager.");
        return false;
    }
    return cloudDrive_manager_->iCloudEnabled;
}
bool        CoqSystem_cloudDrive_isUpdating(void) {
    if(cloudDrive_manager_ == nil) {
        printwarning("No cloud drive manager.");
        return false;
    }
    return cloudDrive_manager_->updating;
}
void        CoqSystem_cloudDrive_stopWatching_(void) {
    if(cloudDrive_manager_ == nil) {
        printwarning("No cloud drive manager.");
        return;
    }
    [cloudDrive_manager_ stopDataQuery];
}


#pragma mark - User Name
/*
char* appleid_givenName_ = NULL;
const char* container_name_ = NULL;
const char* CoqSystem_cloudUserNameOpt(void) {
    return appleid_givenName_;
}
void        CoqSystem_initCloudUserName(const char* container_name_cst) {
    if(appleid_givenName_) {
        printwarning("User name already init.");
        return;
    }
    container_name_ = container_name_cst;
    if([[NSFileManager defaultManager] ubiquityIdentityToken] == nil) {
        printdebug("No iCloud.");
        return;
    }
    CKContainer* ckcontainer = [CKContainer containerWithIdentifier:[NSString stringWithUTF8String:container_name_cst]];
    if(ckcontainer == nil) {
        printerror("Cannot init CKContainer for %s.", container_name_cst);
        return;
    }
    [ckcontainer fetchUserRecordIDWithCompletionHandler:^(CKRecordID * _Nullable record, NSError * _Nullable error) {
        if(record == nil || error != nil) {
            printerror("Cannot get record: %s.", [[error localizedDescription] UTF8String]);
            return;
        }
        [ckcontainer discoverUserIdentityWithUserRecordID:record completionHandler:^(CKUserIdentity * _Nullable_result userInfo, NSError * _Nullable error) {
            if(userInfo == nil) {
                printwarning("Cannot get user info.");
                return;
            }
            if(error != nil) {
                printerror("Error : %s", [[error localizedDescription] UTF8String]);
                return;
            }
            appleid_givenName_ = String_createCopy([[[userInfo nameComponents] givenName] UTF8String]);
            CoqEvent_addToRootEvent((CoqEvent) {.type = event_type_systemChanged, .system_change = { .userNameDidChange = true } });
        }];
    }];
}
void        CoqSystem_requestPermissionAndSetCloudUserName(void) {
    if(appleid_givenName_) {
        printwarning("User name already init.");
        // On revoie tout de meme la réponse pour que ce soit mis a jour...
        CoqEvent_addToRootEvent((CoqEvent) {.type = event_type_systemChanged, .system_change = { .userNameDidChange = true } });
        return;
    }
    if([[NSFileManager defaultManager] ubiquityIdentityToken] == nil) {
        printdebug("No iCloud.");
        return;
    }
    CKContainer* ckcontainer = [CKContainer containerWithIdentifier:[NSString stringWithUTF8String:container_name_]];
    if(ckcontainer == nil) {
        printerror("Cannot init CKContainer for %s.", container_name_);
        return;
    }
    [ckcontainer requestApplicationPermission:CKApplicationPermissionUserDiscoverability
                            completionHandler:^(CKApplicationPermissionStatus permissionStatus, NSError * _Nullable error) {
        if(permissionStatus != CKApplicationPermissionStatusGranted) {
            printerror("Cannot get permission for %s, permission is %ld, error : %s",
                       container_name_,
                       (long)permissionStatus,
                       [[error localizedDescription] UTF8String]);
            return;
        }
        printdebug("Ok, get permission, asking for record...");
        [ckcontainer fetchUserRecordIDWithCompletionHandler:^(CKRecordID * _Nullable record, NSError * _Nullable error) {
            if(record == nil) {
                printwarning("Cannot get user info.");
                return;
            }
            if(error != nil) {
                printerror("Error : %s", [[error localizedDescription] UTF8String]);
                return;
            }
            printdebug("Ok, get record, asking for identity...");
            [ckcontainer discoverUserIdentityWithUserRecordID:record completionHandler:^(CKUserIdentity * _Nullable_result userInfo, NSError * _Nullable error) {
                if(userInfo == nil) {
                    printwarning("Cannot get user info.");
                    return;
                }
                if(error != nil) {
                    printerror("Error : %s", [[error localizedDescription] UTF8String]);
                    return;
                }
                printdebug("User name of current AppleID %s, %s.",
                           [[[userInfo nameComponents] givenName] UTF8String],
                           [[[userInfo nameComponents] familyName] UTF8String]);
                appleid_givenName_ = String_createCopy([[[userInfo nameComponents] givenName] UTF8String]);
                printdebug("Sending system_change event pour userNameDidChange...");
                CoqEvent_addToRootEvent((CoqEvent) {.type = event_type_systemChanged, .system_change = { .userNameDidChange = true } });
            }];
        }];
    }];
}
*/
#pragma mark - Resize or not on virtual keyboard

bool CoqSystem_dontResizeOnVirtualKeyboard = false;


