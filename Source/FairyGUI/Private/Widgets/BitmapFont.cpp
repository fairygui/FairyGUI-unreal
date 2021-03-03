#include "Widgets/BitmapFont.h"

void FBitmapFont::AddReferencedObjects(FReferenceCollector& Collector)
{
    if (Texture != nullptr)
        Collector.AddReferencedObject(Texture);
}