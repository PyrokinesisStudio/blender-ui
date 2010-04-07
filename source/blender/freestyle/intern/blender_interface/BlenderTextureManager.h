#ifndef  BLENDERTEXTUREMANAGER_H
#define BLENDERTEXTUREMANAGER_H

# include "../stroke/StrokeRenderer.h"
# include "../stroke/StrokeRep.h"
# include "../system/FreestyleConfig.h"

/*! Class to load textures
 */
class LIB_RENDERING_EXPORT BlenderTextureManager : public TextureManager
{
 public:
  BlenderTextureManager ();
  virtual ~BlenderTextureManager ();
protected:
  virtual unsigned loadBrush(string fileName, Stroke::MediumType = Stroke::OPAQUE_MEDIUM);
  
 protected:
  virtual void loadStandardBrushes();

};

#endif // BLENDERTEXTUREMANAGER_H
