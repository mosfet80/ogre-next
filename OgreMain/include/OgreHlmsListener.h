/*
-----------------------------------------------------------------------------
This source file is part of OGRE-Next
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef _OgreHlmsListener_H_
#define _OgreHlmsListener_H_

#include "OgreHlmsCommon.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class CompositorShadowNode;
    struct QueuedRenderable;

    /** \addtogroup Core
     *  @{
     */
    /** \addtogroup Resources
     *  @{
     */

    /** Listener that can be hooked to an Hlms implementation for extending it with custom
        code. See "8.6.5 Customizing an existing implementation" of the 2.x manual
        on the different approaches to customizing Hlms implementations.
    @remarks
        For performance reasons, the listener interface does not allow you to add
        customizations that work per Renderable, as that loop is performance sensitive.
        The only listener callback that works inside Hlms::fillBuffersFor is hlmsTypeChanged
        which only gets evaluated when the previous Renderable used a different Hlms
        implementation; which is rare, and since we sort the RenderQueue, it often branch
        predicts well.
    */
    class _OgreExport HlmsListener
    {
    public:
        /** Similar to HlmsListener::shaderCacheEntryCreated, but it gets called before creating
            any shader. The main difference is that there is no hlmsCacheEntry (because it hasn't been
            generated yet) and the properties are before they are transformed by the templates
        @brief propertiesMergedPreGenerationStep
        @param hlms
            @parblock
            Pointer to caller.
            WARNING: Note that any modified property WON'T BE CACHED. If you set a property based
            on external information, it will break caches.

            You can only set new properties that are derived from existing properties e.g. c = a + b,
            which means caching a and b will always result in c being the same value
            @endparblock
        @param passCache
            Properties used by this pass
        @param renderableCacheProperties
            Properties assigned to the renderable
        @param renderableCachePieces
            Pieces that can be inserted, belonging to the renderable.
            The PiecesMap pointer cannot be null
        @param properties
            Combined properties of both renderableCacheProperties & passCache.setProperties
        @param queuedRenderable
        @param tid
            Thread Idx
        */
        virtual void propertiesMergedPreGenerationStep(
            Hlms *hlms, const HlmsCache &passCache, const HlmsPropertyVec &renderableCacheProperties,
            const PiecesMap renderableCachePieces[NumShaderTypes], const HlmsPropertyVec &properties,
            const QueuedRenderable &queuedRenderable, size_t tid )
        {
        }

        /** If hlmsTypeChanged is going to be binding extra textures, override this function
            to tell us how many textures you will use, so that we don't use those slots

            May be called from any thread.
        @remarks
            Given the same set of properties, the function must always return the same
            value. Otherwise caching (i.e. HlmsDiskCache) won't work correctly
        */
        virtual uint16 getNumExtraPassTextures( const HlmsPropertyVec &properties,
                                                bool                   casterPass ) const
        {
            return 0u;
        }

        /** Called right before compiling. If customizations require additional resources slots
            (e.g. more textures) then this function should modify the root layout accordingly

            To allow caches to work, the rootLayout must only be generated from values in 'properties'
            (or via constant modifications) otherwise there is no guarantee a cached version can
            recreate the same state.

            This is why the function is marked const.
        @param rootLayout
            An already-filled rootLayout derived classes can modify
        @param properties
            The current contents of Hlms::mSetProperties
        @param tid
            Thread Idx
        */
        virtual void setupRootLayout( RootLayout &rootLayout, const HlmsPropertyVec &properties,
                                      size_t tid ) const
        {
        }

        /** Called after the shader was created/compiled, and right before
            bindGpuProgramParameters (relevant information for OpenGL programs).
        @param shaderProfile
            See Hlms::mShaderProfile
        @param hlmsCacheEntry
            The created shader.
        @param passCache
            See Hlms::createShaderCacheEntry
        @param properties
            The current contents of Hlms::mSetProperties
        @param queuedRenderable
            See Hlms::createShaderCacheEntry
        @param tid
            Thread Idx
        */
        virtual void shaderCacheEntryCreated( const String           &shaderProfile,
                                              const HlmsCache        *hlmsCacheEntry,
                                              const HlmsCache        &passCache,
                                              const HlmsPropertyVec  &properties,
                                              const QueuedRenderable &queuedRenderable, size_t tid )
        {
        }

        /** Called right before creating the pass cache, to allow the listener
            to add/remove properties.
        @remarks
            For the rest of the parameters, @see Hlms::preparePassHash

            The Thread Idx is always Hlms::kNoTid
        @param hlms
            Caller Hlms; from which you can alter the properties using Hlms::setProperty
        */
        virtual void preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                      bool dualParaboloid, SceneManager *sceneManager, Hlms *hlms )
        {
        }

        /** Listeners should return the number of extra bytes they wish to allocate for storing
            additional data in the pass buffer.

            The actual data will be provided by a preparePassBuffer() override.

            The Thread Idx is always Hlms::kNoTid
        @return
            Number of bytes of additional pass buffer data to allocate.  Must be a multiple of 16.
        */
        virtual uint32 getPassBufferSize( const CompositorShadowNode *shadowNode, bool casterPass,
                                          bool dualParaboloid, SceneManager *sceneManager ) const
        {
            return 0;
        }

        /** Write additional data in the pass buffer.

            Users can write to passBufferPtr. Implementations must ensure they make the buffer
            big enough via getPassBufferSize(). The passBufferPtr is already aligned to 16 bytes.

            The Thread Idx is always Hlms::kNoTid
        @return
            Pointer past the end of added data, aligned to 16 bytes.  The difference between
            the result pointer and the input passBufferPtr must equal the value that you returned from
            getPassBufferSize().
        */
        virtual float *preparePassBuffer( const CompositorShadowNode *shadowNode, bool casterPass,
                                          bool dualParaboloid, SceneManager *sceneManager,
                                          float *passBufferPtr )
        {
            return passBufferPtr;
        }

        /// Called when the last Renderable processed was of a different Hlms type, thus we
        /// need to rebind certain buffers (like the pass buffer). You can use
        /// this moment to bind your own buffers.
        ///
        /// The Thread Idx is always Hlms::kNoTid
        virtual void hlmsTypeChanged( bool casterPass, CommandBuffer *commandBuffer,
                                      const HlmsDatablock *datablock, size_t texUnit )
        {
        }

        /** Called from Hlms::createShaderCacheEntry after an HlmsPso is initialized. It allows to modify
            its currently assigned macroblock. This allows to override the macroblocks for all
            renderables within a pass. The listener can set a property or multiple properties in
            preparePassHash which can be accessed from this method to modify input macroblock.
            @see Hlms::applyStrongMacroblockRules.
        @remarks
            Given the same set of properties, the function must always return the same
            value. Otherwise caching (i.e. HlmsDiskCache) won't work correctly.
        @param macroblock [in/out]
            Current macroblock. You can modify it if needed.
        @param properties
        */
        virtual void applyStrongMacroblockRules( HlmsMacroblock        &macroblock,
                                                 const HlmsPropertyVec &properties ) const
        {
        }

        /// @copydoc HlmsListener::applyStrongMacroblockRules()
        virtual void applyStrongBlendblockRules( HlmsBlendblock        &blendblock,
                                                 const HlmsPropertyVec &properties ) const
        {
        }
    };

    /** @} */
    /** @} */

}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
