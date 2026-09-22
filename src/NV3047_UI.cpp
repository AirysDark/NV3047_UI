#include <Arduino.h>
#include <Config.h>
#include <stdio.h>
#include "NV3047_UI.h"

struct Glyph { const uint8_t* bitmap; uint8_t width, height, advance; };
static const uint8_t FONT[][5] = {
 {0,0,0,0,0},
 {0x7E,0x09,0x09,0x7E,0},{0x7F,0x49,0x49,0x36,0},{0x3E,0x41,0x41,0x22,0},{0x7F,0x41,0x41,0x3E,0},{0x7F,0x49,0x49,0x41,0},{0x7F,0x09,0x09,0x01,0},{0x3E,0x41,0x51,0x32,0},{0x7F,0x08,0x08,0x7F,0},{0x41,0x7F,0x41,0,0},{0x20,0x40,0x41,0x3F,0},{0x7F,0x08,0x14,0x63,0},{0x7F,0x40,0x40,0x40,0},{0x7F,0x06,0x06,0x7F,0},{0x7F,0x06,0x18,0x7F,0},{0x3E,0x41,0x41,0x3E,0},{0x7F,0x09,0x09,0x06,0},{0x3E,0x41,0x61,0x7E,0},{0x7F,0x09,0x19,0x66,0},{0x26,0x49,0x49,0x32,0},{0x01,0x7F,0x01,0x01,0},{0x3F,0x40,0x40,0x3F,0},{0x1F,0x20,0x40,0x3F,0},{0x7F,0x30,0x30,0x7F,0},{0x63,0x14,0x14,0x63,0},{0x07,0x08,0x70,0x07,0},{0x61,0x51,0x49,0x47,0},
 {0x3E,0x51,0x49,0x45,0x3E},{0,0x42,0x7F,0x40,0},{0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},{0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},{0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E},
 {0,0x60,0x60,0,0},{0,0x36,0x36,0,0},{0x60,0x18,0x06,0x01,0},{0x63,0x13,0x08,0x64,0x63},{0x08,0x08,0x3E,0x08,0x08},{0x40,0x40,0x40,0x40,0x40},{0,0,0x5F,0,0},{0x02,0x01,0x51,0x09,0x06},{0x08,0x08,0x08,0x08,0},{0x14,0x14,0x14,0x14,0x14}
};
static Glyph GLYPHS[47];
static bool glyphInit=false;
static const Glyph* glyphFor(char c) {
    if (!glyphInit) { for (uint8_t i=0;i<47;i++) GLYPHS[i]={FONT[i],5,7,6}; glyphInit=true; }
    if (c>='a'&&c<='z') c=(char)(c-'a'+'A');
    if (c==' ') return &GLYPHS[0];
    if (c>='A'&&c<='Z') return &GLYPHS[(c-'A')+1];
    if (c>='0'&&c<='9') return &GLYPHS[27+(c-'0')];
    switch(c){case '.':return &GLYPHS[37];case ':':return &GLYPHS[38];case '/':return &GLYPHS[39];case '%':return &GLYPHS[40];case '+':return &GLYPHS[41];case '_':return &GLYPHS[42];case '!':return &GLYPHS[43];case '?':return &GLYPHS[44];case '-':return &GLYPHS[45];case '=':return &GLYPHS[46];default:return &GLYPHS[44];}
}

UITheme UITheme::dark(){return {UIColor::rgb565(12,16,24),UIColor::rgb565(24,30,42),UIColor::rgb565(35,43,58),UIColor::rgb565(64,75,94),UIColor::rgb565(0,170,255),UIColor::rgb565(0,125,205),UIColor::rgb565(245,248,252),UIColor::rgb565(150,161,179),UIColor::rgb565(44,201,128),UIColor::rgb565(255,184,77),UIColor::rgb565(245,80,92),UIColor::rgb565(5,8,13)};}
UITheme UITheme::light(){return {UIColor::rgb565(238,242,247),UIColor::rgb565(255,255,255),UIColor::rgb565(224,231,239),UIColor::rgb565(190,201,214),UIColor::rgb565(0,122,204),UIColor::rgb565(0,92,160),UIColor::rgb565(25,31,40),UIColor::rgb565(94,106,122),UIColor::rgb565(27,158,99),UIColor::rgb565(215,139,23),UIColor::rgb565(210,58,68),UIColor::rgb565(190,197,207)};}

NV3047_Adapter::NV3047_Adapter()
    : highLevelDriver(nullptr), rawHardware(nullptr) {}

Framebuffer* NV3047_Adapter::canvas() {
    if (highLevelDriver) return highLevelDriver->getCanvas();
    return rawHardware ? &rawHardware->getCanvas() : nullptr;
}

const Framebuffer* NV3047_Adapter::canvas() const {
    if (highLevelDriver) return highLevelDriver->getCanvas();
    return rawHardware ? &rawHardware->getCanvas() : nullptr;
}

bool NV3047_Adapter::begin(NV3047* hw, bool initializeHardware) {
    highLevelDriver = nullptr;
    rawHardware = nullptr;
    if (!hw) return false;

    if (initializeHardware) {
        if (!ownedDriver.begin(hw)) return false;
        highLevelDriver = &ownedDriver;
    } else {
        // Backward-compatible path for applications that already initialized
        // NV3047 directly. The preferred v2 path is initializeHardware=true.
        rawHardware = hw;
        Framebuffer* fb = canvas();
        if (!fb || !fb->isReady()) {
            rawHardware = nullptr;
            return false;
        }
    }
    return true;
}

bool NV3047_Adapter::begin(NV3047_Driver* readyDriver) {
    highLevelDriver = nullptr;
    rawHardware = nullptr;
    if (!readyDriver || !readyDriver->isReady()) return false;
    highLevelDriver = readyDriver;
    return true;
}

void NV3047_Adapter::setBrightness(uint8_t percentage) {
    if (percentage > 100) percentage = 100;
    if (highLevelDriver) highLevelDriver->setBrightness(percentage);
    else if (rawHardware) rawHardware->getDisplay().setBrightness(percentage);
}

void NV3047_Adapter::sleep() {
    if (highLevelDriver) highLevelDriver->sleep();
    else if (rawHardware) rawHardware->getDisplay().sleep();
}

void NV3047_Adapter::wake() {
    if (highLevelDriver) highLevelDriver->wake();
    else if (rawHardware) rawHardware->getDisplay().wake();
}

void NV3047_Adapter::clear(uint16_t color) {
    if (highLevelDriver) highLevelDriver->clear(color);
    else { Framebuffer* fb = canvas(); if (fb) fb->clear(color); }
}

bool NV3047_Adapter::commit() {
    if (highLevelDriver) return highLevelDriver->present();
    Framebuffer* fb = canvas();
    return fb && fb->swap();
}

void NV3047_Adapter::drawPixel(int16_t x,int16_t y,uint16_t c) {
    if (highLevelDriver) highLevelDriver->drawPixel(x,y,c);
    else { Framebuffer* fb=canvas(); if(fb) fb->drawPixel(x,y,c); }
}
void NV3047_Adapter::fillRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t c) {
    if (highLevelDriver) highLevelDriver->fillRect(x,y,w,h,c);
    else { Framebuffer* fb=canvas(); if(fb) fb->fillRect(x,y,w,h,c); }
}
void NV3047_Adapter::drawHLine(int16_t x,int16_t y,int16_t w,uint16_t c) {
    if (highLevelDriver) highLevelDriver->drawHLine(x,y,w,c);
    else { Framebuffer* fb=canvas(); if(fb) fb->drawHLine(x,y,w,c); }
}
void NV3047_Adapter::drawVLine(int16_t x,int16_t y,int16_t hh,uint16_t c) {
    if (highLevelDriver) highLevelDriver->drawVLine(x,y,hh,c);
    else { Framebuffer* fb=canvas(); if(fb) fb->drawVLine(x,y,hh,c); }
}
void NV3047_Adapter::drawRect(int16_t x,int16_t y,int16_t w,int16_t hh,uint16_t c) {
    if (highLevelDriver) highLevelDriver->drawRect(x,y,w,hh,c);
    else { Framebuffer* fb=canvas(); if(fb) fb->drawRect(x,y,w,hh,c); }
}
void NV3047_Adapter::drawBitmap(int16_t x,int16_t y,int16_t w,int16_t hh,const uint16_t* b) {
    if (highLevelDriver) highLevelDriver->pushPixels(x,y,w,hh,b);
    else { Framebuffer* fb=canvas(); if(fb) fb->drawBitmap(x,y,w,hh,b); }
}
bool NV3047_Adapter::getTouch(uint16_t& x,uint16_t& y) {
    if (highLevelDriver) return highLevelDriver->getTouch(x,y);
    return rawHardware ? rawHardware->getTouch().getTouch(x,y) : false;
}
uint16_t NV3047_Adapter::width() const { return Config::SCREEN_WIDTH; }
uint16_t NV3047_Adapter::height() const { return Config::SCREEN_HEIGHT; }

uint32_t NV3047_Adapter::frameCount() const {
    const Framebuffer* fb=canvas(); return fb?fb->getFrameCount():0;
}
uint32_t NV3047_Adapter::lastFrameTimeUs() const {
    const Framebuffer* fb=canvas(); return fb?fb->getLastFrameTimeUs():0;
}
float NV3047_Adapter::presentationFPS() const {
    const Framebuffer* fb=canvas(); return fb?fb->getApproxFPS():0.0f;
}
size_t NV3047_Adapter::framebufferCount() const {
    const Framebuffer* fb=canvas(); return fb?fb->getMemoryManager().getBufferCount():0;
}
size_t NV3047_Adapter::framebufferSizeBytes() const {
    const Framebuffer* fb=canvas(); return fb?fb->getMemoryManager().getBufferSizeBytes():0;
}
size_t NV3047_Adapter::framebufferAllocatedBytes() const {
    const Framebuffer* fb=canvas(); return fb?fb->getMemoryManager().getTotalAllocatedBytes():0;
}
size_t NV3047_Adapter::freeManagedMemoryBytes() const {
    const Framebuffer* fb=canvas(); return fb?fb->getMemoryManager().getFreeManagedMemoryBytes():0;
}
size_t NV3047_Adapter::largestFreeManagedMemoryBlockBytes() const {
    const Framebuffer* fb=canvas(); return fb?fb->getMemoryManager().getLargestFreeManagedMemoryBlockBytes():0;
}

TextRenderer::TextRenderer():display(nullptr){} void TextRenderer::begin(DisplayDriverInterface* d){display=d;} int16_t TextRenderer::textHeight(uint8_t s)const{return 7*(s?s:1);} int16_t TextRenderer::textWidth(const char*t,uint8_t s)const{if(!t||!s)return 0;int16_t line=0,max=0;while(*t){if(*t=='\n'){if(line>max)max=line;line=0;}else line+=glyphFor(*t)->advance*s;++t;}if(line>max)max=line;return max?max-s:0;}
void TextRenderer::drawText(const char*t,int16_t x,int16_t y,uint16_t c,uint8_t s){if(!display||!t||!s)return;int16_t sx=x;while(*t){if(*t=='\n'){x=sx;y+=8*s;++t;continue;}const Glyph*g=glyphFor(*t);for(uint8_t col=0;col<g->width;col++){uint8_t bits=g->bitmap[col];for(uint8_t row=0;row<g->height;row++)if(bits&(1U<<row)){if(s==1)display->drawPixel(x+col,y+row,c);else display->fillRect(x+col*s,y+row*s,s,s,c);}}x+=g->advance*s;++t;}}
void TextRenderer::drawTextCentered(const char*t,int16_t cx,int16_t y,uint16_t c,uint8_t s){drawText(t,cx-textWidth(t,s)/2,y,c,s);}

void UIDraw::fillSoftRect(DisplayDriverInterface*d,const UIRect&r,uint16_t c,int16_t k){if(!d||r.w<=0||r.h<=0)return;if(k<=0||r.w<=k*2||r.h<=k*2){d->fillRect(r.x,r.y,r.w,r.h,c);return;}d->fillRect(r.x+k,r.y,r.w-k*2,r.h,c);d->fillRect(r.x,r.y+k,r.w,r.h-k*2,c);for(int16_t i=1;i<k;i++){d->drawHLine(r.x+k-i,r.y+i,r.w-(k-i)*2,c);d->drawHLine(r.x+k-i,r.y+r.h-1-i,r.w-(k-i)*2,c);}}
void UIDraw::drawSoftRect(DisplayDriverInterface*d,const UIRect&r,uint16_t c,int16_t k){if(!d||r.w<=1||r.h<=1)return;d->drawHLine(r.x+k,r.y,r.w-k*2,c);d->drawHLine(r.x+k,r.y+r.h-1,r.w-k*2,c);d->drawVLine(r.x,r.y+k,r.h-k*2,c);d->drawVLine(r.x+r.w-1,r.y+k,r.h-k*2,c);for(int16_t i=1;i<=k;i++){d->drawPixel(r.x+k-i,r.y+i,c);d->drawPixel(r.x+r.w-1-k+i,r.y+i,c);d->drawPixel(r.x+k-i,r.y+r.h-1-i,c);d->drawPixel(r.x+r.w-1-k+i,r.y+r.h-1-i,c);}}

Widget::Widget():bounds(),visible(true),enabled(true),pressed(false){} Widget::Widget(const UIRect&r):bounds(r),visible(true),enabled(true),pressed(false){} void Widget::setBounds(int16_t x,int16_t y,int16_t w,int16_t h){bounds=UIRect(x,y,w,h);} void Widget::setBounds(const UIRect&r){bounds=r;} const UIRect&Widget::getBounds()const{return bounds;} void Widget::setVisible(bool v){visible=v;} bool Widget::isVisible()const{return visible;} void Widget::setEnabled(bool v){enabled=v;if(!v)pressed=false;} bool Widget::isEnabled()const{return enabled;} bool Widget::hitTest(int16_t x,int16_t y)const{return visible&&enabled&&bounds.contains(x,y);} void Widget::onTouchDown(int16_t,int16_t){pressed=true;} void Widget::onTouchMove(int16_t,int16_t){} void Widget::onTouchUp(int16_t,int16_t,bool){pressed=false;}

Label::Label():Widget(),value(""),scale(1),align(UITextAlign::Left),customColor(0),useCustomColor(false){enabled=false;} Label::Label(const char*t,const UIRect&r,uint8_t s):Widget(r),value(t?t:""),scale(s?s:1),align(UITextAlign::Left),customColor(0),useCustomColor(false){enabled=false;} void Label::setText(const char*t){value=t?t:"";} void Label::setScale(uint8_t s){scale=s?s:1;} void Label::setAlign(UITextAlign a){align=a;} void Label::setColor(uint16_t c){customColor=c;useCustomColor=true;} void Label::useThemeColor(){useCustomColor=false;}
void Label::render(UIContext&ctx){if(!visible||!ctx.text||!ctx.theme)return;int16_t x=bounds.x,w=ctx.text->textWidth(value,scale);if(align==UITextAlign::Center)x=bounds.x+(bounds.w-w)/2;else if(align==UITextAlign::Right)x=bounds.x+bounds.w-w;int16_t y=bounds.y+(bounds.h-ctx.text->textHeight(scale))/2;ctx.text->drawText(value,x,y,useCustomColor?customColor:ctx.theme->text,scale);}

Button::Button():Widget(),label("BUTTON"),callback(nullptr),callbackData(nullptr),textScale(1),accent(true){} Button::Button(const char*t,const UIRect&r):Widget(r),label(t?t:"BUTTON"),callback(nullptr),callbackData(nullptr),textScale(1),accent(true){} void Button::setLabel(const char*t){label=t?t:"";} void Button::setOnClick(UIButtonCallback f,void*d){callback=f;callbackData=d;} void Button::setTextScale(uint8_t s){textScale=s?s:1;} void Button::setAccent(bool v){accent=v;}
void Button::render(UIContext&ctx){if(!visible||!ctx.display||!ctx.text||!ctx.theme)return;UIRect sh(bounds.x,bounds.y+2,bounds.w,bounds.h);UIDraw::fillSoftRect(ctx.display,sh,ctx.theme->shadow,5);uint16_t fill,border,text;if(!enabled){fill=ctx.theme->surfaceAlt;border=ctx.theme->border;text=ctx.theme->muted;}else{fill=accent?(pressed?ctx.theme->primaryPressed:ctx.theme->primary):(pressed?ctx.theme->border:ctx.theme->surfaceAlt);border=accent?ctx.theme->primaryPressed:ctx.theme->border;text=ctx.theme->text;}UIDraw::fillSoftRect(ctx.display,bounds,fill,5);UIDraw::drawSoftRect(ctx.display,bounds,border,5);int16_t y=bounds.y+(bounds.h-ctx.text->textHeight(textScale))/2;ctx.text->drawTextCentered(label,bounds.x+bounds.w/2,y,text,textScale);} void Button::onTouchUp(int16_t,int16_t,bool inside){bool fire=pressed&&inside&&enabled;pressed=false;if(fire&&callback)callback(callbackData);}

Toggle::Toggle():Widget(),label("TOGGLE"),value(false),callback(nullptr),callbackData(nullptr){} Toggle::Toggle(const char*t,const UIRect&r,bool v):Widget(r),label(t?t:"TOGGLE"),value(v),callback(nullptr),callbackData(nullptr){} void Toggle::setValue(bool v,bool notify){if(value==v)return;value=v;if(notify&&callback)callback(value,callbackData);} bool Toggle::getValue()const{return value;} void Toggle::setOnChange(UIToggleCallback f,void*d){callback=f;callbackData=d;}
void Toggle::render(UIContext&ctx){if(!visible||!ctx.display||!ctx.text||!ctx.theme)return;const int16_t sw=52,sh=26,sx=bounds.x+bounds.w-sw,sy=bounds.y+(bounds.h-sh)/2;uint16_t text=enabled?ctx.theme->text:ctx.theme->muted;ctx.text->drawText(label,bounds.x,bounds.y+(bounds.h-7)/2,text,1);UIRect track(sx,sy,sw,sh);uint16_t tc=enabled?(value?ctx.theme->primary:ctx.theme->surfaceAlt):ctx.theme->surfaceAlt;UIDraw::fillSoftRect(ctx.display,track,tc,9);UIDraw::drawSoftRect(ctx.display,track,enabled?(value?ctx.theme->primaryPressed:ctx.theme->border):ctx.theme->border,9);UIRect knob(value?sx+sw-22:sx+4,sy+4,18,18);UIDraw::fillSoftRect(ctx.display,knob,enabled?ctx.theme->text:ctx.theme->muted,6);} void Toggle::onTouchUp(int16_t,int16_t,bool inside){bool fire=pressed&&inside&&enabled;pressed=false;if(fire){value=!value;if(callback)callback(value,callbackData);}}

ProgressBar::ProgressBar():Widget(),label(""),value(0),showValue(true),customFill(0),useCustomFill(false){enabled=false;} ProgressBar::ProgressBar(const char*t,const UIRect&r,uint8_t v):Widget(r),label(t?t:""),value(v>100?100:v),showValue(true),customFill(0),useCustomFill(false){enabled=false;} void ProgressBar::setValue(uint8_t v){value=v>100?100:v;} uint8_t ProgressBar::getValue()const{return value;} void ProgressBar::setShowValue(bool v){showValue=v;} void ProgressBar::setFillColor(uint16_t c){customFill=c;useCustomFill=true;} void ProgressBar::useThemeFill(){useCustomFill=false;}
void ProgressBar::render(UIContext&ctx){if(!visible||!ctx.display||!ctx.text||!ctx.theme)return;ctx.text->drawText(label,bounds.x,bounds.y,ctx.theme->muted,1);if(showValue){char b[6];snprintf(b,sizeof(b),"%u%%",(unsigned)value);ctx.text->drawText(b,bounds.x+bounds.w-ctx.text->textWidth(b),bounds.y,ctx.theme->text,1);}UIRect tr(bounds.x,bounds.y+17,bounds.w,bounds.h-17);if(tr.h<8)tr.h=8;UIDraw::fillSoftRect(ctx.display,tr,ctx.theme->surfaceAlt,3);UIDraw::drawSoftRect(ctx.display,tr,ctx.theme->border,3);int16_t iw=tr.w>4?tr.w-4:0,fw=(int16_t)((int32_t)iw*value/100);if(fw>0){UIRect f(tr.x+2,tr.y+2,fw,tr.h-4);UIDraw::fillSoftRect(ctx.display,f,useCustomFill?customFill:ctx.theme->primary,2);}}

Slider::Slider():Widget(),label(""),value(0),callback(nullptr),callbackData(nullptr){} Slider::Slider(const char*t,const UIRect&r,uint8_t v):Widget(r),label(t?t:""),value(v>100?100:v),callback(nullptr),callbackData(nullptr){} void Slider::setValue(uint8_t v,bool notify){if(v>100)v=100;if(value==v)return;value=v;if(notify&&callback)callback(value,callbackData);} uint8_t Slider::getValue()const{return value;} void Slider::setOnChange(UISliderCallback f,void*d){callback=f;callbackData=d;} void Slider::updateFromX(int16_t x,bool notify){int16_t l=bounds.x+8,r=bounds.x+bounds.w-8;if(r<=l)return;if(x<l)x=l;if(x>r)x=r;setValue((uint8_t)(((int32_t)(x-l)*100)/(r-l)),notify);}
void Slider::render(UIContext&ctx){if(!visible||!ctx.display||!ctx.text||!ctx.theme)return;uint16_t tx=enabled?ctx.theme->text:ctx.theme->muted;ctx.text->drawText(label,bounds.x,bounds.y,ctx.theme->muted,1);char b[5];snprintf(b,sizeof(b),"%u",(unsigned)value);ctx.text->drawText(b,bounds.x+bounds.w-ctx.text->textWidth(b),bounds.y,tx,1);int16_t y=bounds.y+bounds.h-12,l=bounds.x+8,w=bounds.w-16;ctx.display->fillRect(l,y,w,4,ctx.theme->surfaceAlt);int16_t fw=(int16_t)((int32_t)w*value/100);if(fw>0)ctx.display->fillRect(l,y,fw,4,enabled?ctx.theme->primary:ctx.theme->muted);UIRect k(l+fw-5,y-5,10,14);UIDraw::fillSoftRect(ctx.display,k,enabled?(pressed?ctx.theme->primaryPressed:ctx.theme->text):ctx.theme->muted,3);} void Slider::onTouchDown(int16_t x,int16_t){pressed=true;updateFromX(x,true);} void Slider::onTouchMove(int16_t x,int16_t){if(pressed)updateFromX(x,true);} void Slider::onTouchUp(int16_t x,int16_t,bool){if(pressed)updateFromX(x,true);pressed=false;}

Screen::Screen():widgetCount(0),focusedWidget(nullptr),initialized(false){for(uint8_t i=0;i<MAX_WIDGETS;i++){widgets[i].widget=nullptr;widgets[i].z=0;}} bool Screen::add(Widget&w,uint8_t z){return registerWidget(&w,z);} bool Screen::registerWidget(Widget*w,uint8_t z){if(!w||widgetCount>=MAX_WIDGETS)return false;uint8_t at=widgetCount;for(uint8_t i=0;i<widgetCount;i++)if(widgets[i].z>z){at=i;break;}for(uint8_t i=widgetCount;i>at;i--)widgets[i]=widgets[i-1];widgets[at]={w,z};widgetCount++;return true;} bool Screen::remove(Widget&w){return unregisterWidget(&w);} bool Screen::unregisterWidget(Widget*w){if(!w)return false;for(uint8_t i=0;i<widgetCount;i++)if(widgets[i].widget==w){if(focusedWidget==w)focusedWidget=nullptr;for(uint8_t j=i;j+1<widgetCount;j++)widgets[j]=widgets[j+1];widgetCount--;widgets[widgetCount].widget=nullptr;return true;}return false;} void Screen::clearWidgets(){widgetCount=0;focusedWidget=nullptr;} void Screen::focus(){if(!initialized){onInit();initialized=true;}onEnter();} void Screen::defocus(){focusedWidget=nullptr;onExit();}
void Screen::handleInput(uint16_t x,uint16_t y){if(focusedWidget){focusedWidget->onTouchMove(x,y);return;}for(int i=(int)widgetCount-1;i>=0;i--){Widget*w=widgets[i].widget;if(w&&w->hitTest(x,y)){focusedWidget=w;w->onTouchDown(x,y);break;}}} void Screen::releaseInput(uint16_t x,uint16_t y){if(!focusedWidget)return;Widget*w=focusedWidget;focusedWidget=nullptr;w->onTouchUp(x,y,w->hitTest(x,y));} void Screen::processInput(uint16_t x,uint16_t y,bool t){if(t)handleInput(x,y);else releaseInput(x,y);} void Screen::render(UIContext&ctx){onDraw(ctx);for(uint8_t i=0;i<widgetCount;i++)if(widgets[i].widget&&widgets[i].widget->isVisible())widgets[i].widget->render(ctx);}

NV3047_UI::NV3047_UI()
    : display(nullptr), activeScreen(nullptr), activeTheme(UITheme::dark()),
      lastInteractionTime(0), frameCounter(0), fpsWindowStart(0),
      measuredFps(0), lastPresentOK(false), wasTouched(false),
      lastTouchX(0), lastTouchY(0) {}

bool NV3047_UI::begin(DisplayDriverInterface* d) {
    if (!d) return false;
    display=d;
    textRenderer.begin(d);
    uint32_t now=millis();
    lastInteractionTime=now;
    fpsWindowStart=now;
    frameCounter=0;
    measuredFps=0;
    lastPresentOK=true;
    wasTouched=false;
    return true;
}

bool NV3047_UI::begin(NV3047* h,bool init) {
    if(!nvAdapter.begin(h,init)) return false;
    return begin(static_cast<DisplayDriverInterface*>(&nvAdapter));
}

bool NV3047_UI::begin(NV3047_Driver* d) {
    if(!nvAdapter.begin(d)) return false;
    return begin(static_cast<DisplayDriverInterface*>(&nvAdapter));
}

void NV3047_UI::loadScreen(Screen* s) {
    if(activeScreen==s) return;
    if(activeScreen) activeScreen->defocus();
    activeScreen=s;
    if(activeScreen) activeScreen->focus();
}
Screen* NV3047_UI::getActiveScreen()const{return activeScreen;}
void NV3047_UI::setTheme(const UITheme&t){activeTheme=t;}
const UITheme& NV3047_UI::theme()const{return activeTheme;}
void NV3047_UI::setBackgroundColor(uint16_t c){activeTheme.background=c;}

void NV3047_UI::setBrightness(uint8_t percentage) {
    if(percentage>100) percentage=100;
    if(display==&nvAdapter) nvAdapter.setBrightness(percentage);
}
void NV3047_UI::sleep(){if(display==&nvAdapter)nvAdapter.sleep();}
void NV3047_UI::wake(){if(display==&nvAdapter)nvAdapter.wake();}

uint16_t NV3047_UI::width()const{return display?display->width():0;}
uint16_t NV3047_UI::height()const{return display?display->height():0;}
uint32_t NV3047_UI::getIdleTimeMs()const{return millis()-lastInteractionTime;}
uint16_t NV3047_UI::getFPS()const{return measuredFps;}
float NV3047_UI::getPresentationFPS()const{return display?display->presentationFPS():0.0f;}

bool NV3047_UI::getDriverStats(UIDriverStats& stats) const {
    if(!display) return false;
    stats.frameCount=display->frameCount();
    stats.lastFrameTimeUs=display->lastFrameTimeUs();
    stats.presentationFPS=display->presentationFPS();
    stats.framebufferCount=display->framebufferCount();
    stats.framebufferSizeBytes=display->framebufferSizeBytes();
    stats.framebufferAllocatedBytes=display->framebufferAllocatedBytes();
    stats.freeManagedMemoryBytes=display->freeManagedMemoryBytes();
    stats.largestFreeManagedMemoryBlockBytes=display->largestFreeManagedMemoryBlockBytes();
    return true;
}

bool NV3047_UI::lastPresentSucceeded()const{return lastPresentOK;}
TextRenderer& NV3047_UI::text(){return textRenderer;}
DisplayDriverInterface* NV3047_UI::driver(){return display;}
void NV3047_UI::drawText(const char*v,int16_t x,int16_t y,uint16_t c,uint8_t s){textRenderer.drawText(v,x,y,c,s);}

bool NV3047_UI::update() {
    if(!display){lastPresentOK=false;return false;}

    uint16_t x=lastTouchX,y=lastTouchY;
    bool touched=display->getTouch(x,y);
    if(activeScreen){
        if(touched){
            activeScreen->handleInput(x,y);
            lastInteractionTime=millis();
            lastTouchX=x;
            lastTouchY=y;
        } else if(wasTouched){
            activeScreen->releaseInput(lastTouchX,lastTouchY);
            lastInteractionTime=millis();
        }
    }

    display->clear(activeTheme.background);
    if(activeScreen){
        UIContext ctx={display,&textRenderer,&activeTheme};
        activeScreen->render(ctx);
    }

    lastPresentOK=display->commit();
    wasTouched=touched;

    frameCounter++;
    uint32_t now=millis();
    uint32_t elapsed=now-fpsWindowStart;
    if(elapsed>=1000){
        measuredFps=(uint16_t)((frameCounter*1000UL)/elapsed);
        frameCounter=0;
        fpsWindowStart=now;
    }

    return lastPresentOK;
}

