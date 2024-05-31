//
//  TextStimulus.hpp
//  TextStimulus
//
//  Created by Christopher Stawarz on 9/13/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef TextStimulus_hpp
#define TextStimulus_hpp


BEGIN_NAMESPACE_MW


class TextStimulus : public ColoredTransformStimulus {
    
public:
    static const std::string TEXT;
    static const std::string FONT_NAME;
    static const std::string FONT_SIZE;
    static const std::string TEXT_ALIGNMENT;
    static const std::string MAX_SIZE_X;
    static const std::string MAX_SIZE_Y;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit TextStimulus(const ParameterValueMap &parameters);
    ~TextStimulus();
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    void updateTexture(MetalDisplay &display);
    
    const VariablePtr text;
    const VariablePtr fontName;
    const VariablePtr fontSize;
    const VariablePtr textAlignment;
    const VariablePtr maxSizeX;
    const VariablePtr maxSizeY;
    
    float currentMaxSizeX, currentMaxSizeY;
    double pixelsPerPoint;
    
    std::size_t textureWidth, textureHeight;
    std::size_t textureBytesPerRow;
    std::unique_ptr<std::uint8_t[]> textureData;
    
    cf::ObjectPtr<CGContextRef> context;
    
    MWKTripleBufferedMTLResource<id<MTLTexture>> *texturePool;
    id<MTLTexture> currentTexture;
    
    std::size_t currentWidthPixels, currentHeightPixels;
    std::string currentText, currentFontName;
    CGFloat currentFontSize;
    std::string currentTextAlignment;
    
    std::size_t lastWidthPixels, lastHeightPixels;
    std::string lastText, lastFontName;
    CGFloat lastFontSize;
    std::string lastTextAlignment;
    
};


END_NAMESPACE_MW


#endif /* TextStimulus_hpp */
