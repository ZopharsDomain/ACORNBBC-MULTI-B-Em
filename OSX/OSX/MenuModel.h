/* MenuModel */

#import <Cocoa/Cocoa.h>

@interface MenuModel : NSObject
{
    IBOutlet NSButton *blurFilterButton;
    IBOutlet NSButton *discNoisesButton;
    IBOutlet NSButton *fastTapeButton;
    IBOutlet NSButton *highPassFilterButton;
    IBOutlet NSButton *lowPassFilterButton;
    IBOutlet NSMatrix *modelList;
    IBOutlet NSButton *monochromeButton;
    IBOutlet NSMatrix *outputList;
    IBOutlet NSWindow *preferencesWindow;
    IBOutlet NSButton *soundEnableButton;
    IBOutlet NSButton *autobootButton;
    IBOutlet NSPopUpButton *waveformPopUpButton;
}
- (IBAction)loaddisc1:(id)sender;
- (IBAction)loaddisc2:(id)sender;
- (IBAction)loadstate:(id)sender;
- (IBAction)loadtape:(id)sender;
- (IBAction)savestate:(id)sender;
- (IBAction)setBlurFilter:(id)sender;
- (IBAction)setDiscNoises:(id)sender;
- (IBAction)setFastTape:(id)sender;
- (IBAction)setHighPassFilter:(id)sender;
- (IBAction)setLowPassFilter:(id)sender;
- (IBAction)setModel:(id)sender;
- (IBAction)setAutoboot:(id)sender;
- (IBAction)setMonochrome:(id)sender;
- (IBAction)setOutput:(id)sender;
- (IBAction)setSoundEnable:(id)sender;
- (IBAction)setWaveform:(id)sender;
- (IBAction)showPreferences:(id)sender;
- (IBAction)setupFromGlobals;
- (IBAction)resetMachine:(id)sender;
- (IBAction)showHelp:(id)sender;
- (IBAction)showHistory:(id)sender;
- (void)openResourceFile:(NSString *)fileName;
@end
