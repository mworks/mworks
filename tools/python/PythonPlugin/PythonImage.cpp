//
//  PythonImage.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 10/1/20.
//  Copyright © 2020 MWorks Project. All rights reserved.
//

#include "PythonImage.hpp"

#include "ObjectPtr.h"
#include "PythonException.h"


BEGIN_NAMESPACE_MW_PYTHON


const std::string PythonImage::PIXEL_BUFFER_FORMAT("pixel_buffer_format");
const std::string PythonImage::PIXEL_BUFFER_WIDTH("pixel_buffer_width");
const std::string PythonImage::PIXEL_BUFFER_HEIGHT("pixel_buffer_height");
const std::string PythonImage::PIXEL_BUFFER_EXPR("pixel_buffer_expr");
const std::string PythonImage::ELAPSED_TIME("elapsed_time");


void PythonImage::describeComponent(ComponentInfo &info) {
    PythonImageBase::describeComponent(info);
    
    info.setSignature("stimulus/python_image");
    
    info.addParameter(PIXEL_BUFFER_FORMAT);
    info.addParameter(PIXEL_BUFFER_WIDTH);
    info.addParameter(PIXEL_BUFFER_HEIGHT);
    info.addParameter(PIXEL_BUFFER_EXPR);
    info.addParameter(ELAPSED_TIME, false);
    info.addParameter(AUTOPLAY, "YES");  // Change autoplay's default to YES
}


PythonImage::PythonImage(const ParameterValueMap &parameters) :
    PythonImageBase(parameters),
    pixelBufferFormat(variableOrText(parameters[PIXEL_BUFFER_FORMAT])),
    pixelBufferWidth(parameters[PIXEL_BUFFER_WIDTH]),
    pixelBufferHeight(parameters[PIXEL_BUFFER_HEIGHT]),
    pixelBufferExprEvaluator(parameters[PIXEL_BUFFER_EXPR].str(), true),
    elapsedTime(optionalVariable(parameters[ELAPSED_TIME])),
    width(0),
    height(0),
    expectedBytes(0)
{ }


Datum PythonImage::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    Datum announceData = PythonImageBase::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "python_image");
    announceData.addElement(PIXEL_BUFFER_FORMAT, format);
    announceData.addElement(PIXEL_BUFFER_WIDTH, width);
    announceData.addElement(PIXEL_BUFFER_HEIGHT, height);
    
    return announceData;
}


void PythonImage::prepare(const boost::shared_ptr<StimulusDisplay> &display) {
    PythonImageBase::prepare(display);
    
    format = pixelBufferFormat->getValue().getString();
    std::size_t bytesPerPixel = 0;
    if (format == "RGB8" || format == "sRGB8") {
        bytesPerPixel = 3;
        textureFormat = GL_RGB;
        textureInternalFormat = (format.front() == 's' ? GL_SRGB8 : GL_RGB8);
        textureDataType = GL_UNSIGNED_BYTE;
        textureDataAlignment = 1;
    } else if (format == "RGBA8" || format == "sRGBA8") {
        bytesPerPixel = 4;
        textureFormat = GL_RGBA;
        textureInternalFormat = (format.front() == 's' ? GL_SRGB8_ALPHA8 : GL_RGBA8);
        textureDataType = GL_UNSIGNED_BYTE;
        textureDataAlignment = 1;
    } else if (format == "RGBA16F") {
        bytesPerPixel = 8;
        textureFormat = GL_RGBA;
        textureInternalFormat = GL_RGBA16F;
        textureDataType = GL_HALF_FLOAT;
        textureDataAlignment = 2;
    } else {
        throw SimpleException(boost::format("Invalid pixel buffer format: \"%1%\"") % format);
    }
    
    width = pixelBufferWidth->getValue().getInteger();
    height = pixelBufferHeight->getValue().getInteger();
    if (width < 1 || height < 1) {
        throw SimpleException("Pixel buffer width and height must be greater than or equal to one");
    }
    
    expectedBytes = bytesPerPixel * width * height;
}


void PythonImage::preDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    PythonImageBase::preDraw(display);
    
    PythonEvaluator::EvalState es;
    auto pixelBuffer = ObjectPtr::owned(pixelBufferExprEvaluator.eval());
    
    if (!pixelBuffer) {
        PythonException::logError("Pixel buffer expression evaluation failed");
        return;
    }
    if (pixelBuffer.isNone()) {
        // None means the texture doesn't need to be updated
        return;
    }
    if (!PyObject_CheckBuffer(pixelBuffer.get())) {
        merror(M_PLUGIN_MESSAGE_DOMAIN, "Pixel buffer object does not support the Python buffer interface");
        return;
    }
    
    Py_buffer view;
    std::memset(&view, 0, sizeof(view));
    if (-1 == PyObject_GetBuffer(pixelBuffer.get(), &view, PyBUF_SIMPLE)) {
        PythonException::logError("Cannot access pixel buffer data");
        return;
    }
    BOOST_SCOPE_EXIT(&view) {
        PyBuffer_Release(&view);
    } BOOST_SCOPE_EXIT_END
    
    if (!(view.buf && view.len > 0)) {
        merror(M_PLUGIN_MESSAGE_DOMAIN, "Pixel buffer object contains no data");
        return;
    }
    if (view.len != expectedBytes) {
        merror(M_PLUGIN_MESSAGE_DOMAIN,
               "Pixel buffer data contains wrong number of bytes (expected %lu, got %ld)",
               expectedBytes,
               view.len);
        return;
    }
    
    gl::resetPixelStorageUnpackParameters(textureDataAlignment);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 textureInternalFormat,
                 width,
                 height,
                 0,
                 textureFormat,
                 textureDataType,
                 view.buf);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void PythonImage::drawFrame(boost::shared_ptr<StimulusDisplay> display) {
    if (elapsedTime) {
        elapsedTime->setValue(Datum(getElapsedTime()));
    }
    BaseImageStimulus::draw(display);
}


double PythonImage::getAspectRatio() const {
    return double(width) / double(height);
}


END_NAMESPACE_MW_PYTHON
