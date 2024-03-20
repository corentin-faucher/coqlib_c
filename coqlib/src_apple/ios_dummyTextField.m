//
//  DummyTextField.m
//  MasaKiokuGame
//
//  Created by Corentin Faucher on 2024-01-31.
//

#import "ios_dummyTextField.h"

@implementation DummyTextField

-(id)initWithFrame:(CGRect)frame andMetalView:(CoqMetalView *)view {
    self = [super initWithFrame:frame];
    if(self == nil) return nil;
    [self setDelegate:self];
    metalView = view;
    [self setHidden:YES];
    [self setText:@"-"];
    self.borderStyle            = UITextBorderStyleRoundedRect;
    self.autocapitalizationType = UITextAutocapitalizationTypeNone;
    self.spellCheckingType      = UITextSpellCheckingTypeNo;
    self.autocorrectionType     = UITextAutocorrectionTypeNo;
    [self setInputAccessoryView:nil];
    UITextInputAssistantItem* item = [self inputAssistantItem];
    item.leadingBarButtonGroups = @[];
    item.trailingBarButtonGroups = @[];
#warning Utile ? bogue avec le mode dictée... ?
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(inputModeDidChange:)
                                                 name:UITextInputCurrentInputModeDidChangeNotification
                                               object:nil];
    
    return self;
}

-(BOOL)canBecomeFirstResponder { return YES; }
-(BOOL)canResignFirstResponder { return !_locked; }
-(BOOL)becomeFirstResponder {
    [self setLocked:YES];
    return [super becomeFirstResponder];
}

-(void)inputModeDidChange:(NSNotification*)notification {
    if([[self.textInputMode primaryLanguage] isEqualToString:@"dictation"]) {
        [self resignFirstResponder];
    } else {
        [self setText:@"-"];
    }
}

-(BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
    [metalView setPaused:NO];
#warning Inutile ?
    [self setText:@"-"];
    CoqEvent coqevent;
    // Backspace
    if(string.length == 0) {
        coqevent = (CoqEvent){
            .type = event_type_key_down,
            .key = { 0, keycode_delete, mkc_delete, false }
        };
    } else {
        coqevent = (CoqEvent){
            .type = event_type_key_down,
            .key = { 0, keycode__text, mkc__text, false }
        };
        strncpy(coqevent.key.typed.c_str, [string UTF8String], CHARACTER_MAX_SIZE);
    }
    CoqEvent_addToRootEvent(coqevent);
    
    return NO;
}

-(BOOL)textFieldShouldReturn:(UITextField *)textField {
    [metalView setPaused:NO];
    CoqEvent coqevent = {
        .type = event_type_key_down,
        .key = { 0, keycode_return_, mkc_return_, false, spchar_return_ }
    };
    CoqEvent_addToRootEvent(coqevent);
    
    return NO;
}


@end
