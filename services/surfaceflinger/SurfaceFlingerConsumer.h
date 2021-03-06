/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_SURFACEFLINGERCONSUMER_H
#define ANDROID_SURFACEFLINGERCONSUMER_H

#include "DispSync.h"
#include <gui/GLConsumer.h>
#include <gui/BufferQueueCore.h>

namespace android {
// ----------------------------------------------------------------------------

/*
 * This is a thin wrapper around GLConsumer.
 */
class BufferQueueCore;
class SurfaceFlingerConsumer : public GLConsumer {
public:
    struct ContentsChangedListener: public FrameAvailableListener {
        virtual void onSidebandStreamChanged() = 0;
    };

    SurfaceFlingerConsumer(const sp<IGraphicBufferConsumer>& consumer,
            uint32_t tex)
#ifndef MTK_MT6589
        : GLConsumer(consumer, tex, GLConsumer::TEXTURE_EXTERNAL, false, false),
#else
        : GLConsumer(consumer, tex, GLConsumer::TEXTURE_EXTERNAL, false, false), bq (bq),
#endif
          mTransformToDisplayInverse(false)
    {}

    class BufferRejecter {
        friend class SurfaceFlingerConsumer;
        virtual bool reject(const sp<GraphicBuffer>& buf,
                const IGraphicBufferConsumer::BufferItem& item) = 0;

    protected:
        virtual ~BufferRejecter() { }
    };

    virtual status_t acquireBufferLocked(BufferQueue::BufferItem *item, nsecs_t presentWhen);

    // This version of updateTexImage() takes a functor that may be used to
    // reject the newly acquired buffer.  Unlike the GLConsumer version,
    // this does not guarantee that the buffer has been bound to the GL
    // texture.
    status_t updateTexImage(BufferRejecter* rejecter, const DispSync& dispSync);

    // See GLConsumer::bindTextureImageLocked().
    status_t bindTextureImage();

    // must be called from SF main thread
    bool getTransformToDisplayInverse() const;

    // Sets the contents changed listener. This should be used instead of
    // ConsumerBase::setFrameAvailableListener().
    void setContentsChangedListener(const wp<ContentsChangedListener>& listener);

    sp<NativeHandle> getSidebandStream() const;

    nsecs_t computeExpectedPresent(const DispSync& dispSync);
#ifdef MTK_MT6589
    // get connected api type, for buffer data conversion condition (aux and hwc)
    int getConnectedApi();
#endif

private:
    virtual void onSidebandStreamChanged();

    wp<ContentsChangedListener> mContentsChangedListener;

    // Indicates this buffer must be transformed by the inverse transform of the screen
    // it is displayed onto. This is applied after GLConsumer::mCurrentTransform.
    // This must be set/read from SurfaceFlinger's main thread.
    bool mTransformToDisplayInverse;
#ifdef MTK_MT6589
    sp<BufferQueueCore> bq;
#endif
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_SURFACEFLINGERCONSUMER_H
