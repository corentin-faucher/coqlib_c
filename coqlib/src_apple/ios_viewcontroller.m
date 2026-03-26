//
//  ios_viewcontroller.m
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2026-01-11.
//

#import "ios_viewcontroller.h"

#include "systems/system_base.h"
#include "util_apple.h"

@interface ViewControllerBase ()

@end

@implementation ViewControllerBase

- (void)viewDidLoad {
    [super viewDidLoad];
    guard_let(CoqMetalView*, view, uiview_asCoqMetalViewOpt(self.view), 
            printerror("Not a CoqMetalView. Set view in Main.storyboard."),
    )
    view->viewController = self;
    
    CoqSystem_init((ViewSizeInfo) {
        .margins = [view getMargins],
        .framePt = CGRect_toRectangle(view.frame),
        .frameSizePx = CGSize_toVector2(view.drawableSize),
    });
}


@end
