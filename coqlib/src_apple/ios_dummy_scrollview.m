//
//  ios_dummy_scrollview.m
//  coqlib_test_xcode
//
//  Created by Corentin Faucher on 2024-02-01.
//

#import "ios_dummy_scrollview.h"

@implementation DummyScrollView

-(id)initWithFrame:(CGRect)frame contentFactor:(CGFloat)factor offSet:(CGFloat)offSet metalView:(CoqMetalView *)view {
    self = [super initWithFrame:frame];
    if(self == nil) return nil;
    self->metalView = view;
    self.delegate = self;
    
    CGFloat contentHeight = factor * frame.size.height;
    
    [self setContentSize:CGSizeMake(self.bounds.size.width, contentHeight)];
    [self setContentOffset:CGPointMake(self.contentOffset.x, offSet * self.contentSize.height)];
    
    /* Test couleurs de fond...
    CGFloat subviewheight = self.contentSize.height / 8.0;
    CGFloat ypos = 0;
    while(ypos < self.contentSize.height - 5) {
        CGRect frame = CGRectMake(0, ypos, self.contentSize.width, subviewheight);
        CGFloat hue = ypos / self.contentSize.height;
        UIView* subview = [[UIView alloc] initWithFrame:frame];
        subview.backgroundColor = [UIColor colorWithHue:hue saturation:1 brightness:1 alpha:0.35];
        [self addSubview:subview];
        
        ypos += subviewheight;        
    }
    */
    return self;
}

-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [[self nextResponder] touchesBegan:touches withEvent:event];
}
-(void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [[self nextResponder] touchesMoved:touches withEvent:event];
}
-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [[self nextResponder] touchesEnded:touches withEvent:event];
}
-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [[self nextResponder] touchesCancelled:touches withEvent:event];
    
}


-(void)scrollViewDidScroll:(UIScrollView *)scrollView {
    [self->metalView setPaused:NO];
    float offSetRatio = (float)scrollView.contentOffset.y / scrollView.contentSize.height;
    CoqEvent_addToRootEvent((CoqEvent) {
        .type = eventtype_scroll,
        .scroll_info = {.scrollType = scrolltype_offSet, .offset_ratio = offSetRatio, .offset_letGo = false }
    });
}
-(void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate {
    [self->metalView setPaused:NO];
    float offSetRatio = (float)scrollView.contentOffset.y / scrollView.contentSize.height;
    CoqEvent_addToRootEvent((CoqEvent) {
        .type = eventtype_scroll,
        .scroll_info = {.scrollType = scrolltype_offSet, .offset_ratio = offSetRatio, .offset_letGo = true }
    });
}
-(void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView {
    [self->metalView setPaused:NO];
    float offSetRatio = (float)scrollView.contentOffset.y / scrollView.contentSize.height;
    CoqEvent_addToRootEvent((CoqEvent) {
        .type = eventtype_scroll,
        .scroll_info = {.scrollType = scrolltype_offSet, .offset_ratio = offSetRatio, .offset_letGo = true }
    });
}
@end
