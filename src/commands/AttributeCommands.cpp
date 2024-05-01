#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usdGeom/gprim.h>
#include <pxr/usd/sdf/propertySpec.h>
#include <pxr/base/vt/value.h>
#include "CommandsImpl.h"
#include "CommandStack.h"
#include "SdfCommandGroupRecorder.h"

PXR_NAMESPACE_USING_DIRECTIVE

/// Due to static_assert in the attribute.h file, the compiler is not able find the correct UsdAttribute::Set function
/// so we had to create a specific command for this

struct AttributeSet : public SdfLayerCommand {

    AttributeSet(UsdAttribute attribute, VtValue value, UsdTimeCode currentTime)
        : _stage(attribute.GetStage()), _path(attribute.GetPath()), _value(std::move(value)), _timeCode(currentTime) {}

    ~AttributeSet() override {}

    bool DoIt() override {
        if (_stage) {
            auto layer = _stage->GetEditTarget().GetLayer();
            if (layer) {
                SdfCommandGroupRecorder recorder(_undoCommands, layer);
                const UsdAttribute &attribute = _stage->GetAttributeAtPath(_path);
                attribute.Set(_value, _timeCode);
                return true;
            }
        }
        return false;
    }

    UsdStageWeakPtr _stage;
    SdfPath _path;
    VtValue _value;
    UsdTimeCode _timeCode;
};
template void ExecuteAfterDraw<AttributeSet>(UsdAttribute attribute, VtValue value, UsdTimeCode currentTime);


struct AttributeCreateDefaultValue : public SdfLayerCommand {

    AttributeCreateDefaultValue(UsdAttribute attribute)
        : _stage(attribute.GetStage()), _path(attribute.GetPath()) {}

    ~AttributeCreateDefaultValue() override {}

    bool DoIt() override {
        if (_stage) {
            auto layer = _stage->GetEditTarget().GetLayer();
            if (layer) {
                SdfCommandGroupRecorder recorder(_undoCommands, layer);
                const UsdAttribute &attribute = _stage->GetAttributeAtPath(_path);
                if (attribute) {
                    VtValue value = attribute.GetTypeName().GetDefaultValue();
                    if (value.IsHolding<VtArray<GfVec3f>>() && attribute.GetRoleName() == TfToken("Color"))
                        value = VtArray<GfVec3f>{ {0.0,0.0,0.0} };
                    if (value!=VtValue()) {
                        // This will override the default value if there is one already
                        attribute.Set(value, UsdTimeCode::Default());
                        return true;
                    }
                }
            }
        }
        return false;
    }

    UsdStageWeakPtr _stage;
    SdfPath _path;
};
template void ExecuteAfterDraw<AttributeCreateDefaultValue>(UsdAttribute attribute);

struct AttributeConnect : public SdfLayerCommand {
    AttributeConnect(UsdStageWeakPtr stage, SdfPath tail, SdfPath head) :
    _stage(stage), _head(head), _tail(tail) {}
    
    bool DoIt() override {
        if (_stage) {
            auto layer = _stage->GetEditTarget().GetLayer();
            if (layer) {
                SdfCommandGroupRecorder recorder(_undoCommands, layer);
                UsdAttribute headAttr = _stage->GetAttributeAtPath(_head);
                headAttr.AddConnection(_tail);
                return true;
            }
        }
        return false;
    }
    
    UsdStageWeakPtr _stage;
    SdfPath _head;
    SdfPath _tail;
};
template void ExecuteAfterDraw<AttributeConnect>(UsdStageWeakPtr stage, SdfPath tail, SdfPath head);
