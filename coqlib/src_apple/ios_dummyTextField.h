//
//  ios_dummyTextField.h
//  MasaKiokuGame
//
//  Created by Corentin Faucher on 2024-01-31.
//

#ifndef ios_dummyTextField_h
#define ios_dummyTextField_h

#import <UIKit/UIKit.h>
#import "metal_view.h"

@interface DummyTextField : UITextField <UITextFieldDelegate> {
    CoqMetalView* metalView;
}

@property (nonatomic) BOOL locked;

- (instancetype)initWithFrame:(CGRect)frame andMetalView:(CoqMetalView*)metalView;

@end

#endif /* ios_dummyTextField_h */
