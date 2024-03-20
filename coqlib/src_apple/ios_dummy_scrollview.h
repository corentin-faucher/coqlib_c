//
//  ios_dummy_scrollview.h
//  coqlib_test_xcode
//
//  Created by Corentin Faucher on 2024-02-01.
//

#ifndef ios_dummy_scrollview_h
#define ios_dummy_scrollview_h

#import <UIKit/UIKit.h>
#import "metal_view.h"
#include "nodes/node_sliding_menu.h"

@interface DummyScrollView : UIScrollView <UIScrollViewDelegate> {
    CoqMetalView* metalView;
}

-(instancetype)initWithFrame:(CGRect)frame contentFactor:(CGFloat)factor offSet:(CGFloat)offSet metalView:(CoqMetalView *)view;

@end

#endif /* ios_dummy_scrollview_h */
