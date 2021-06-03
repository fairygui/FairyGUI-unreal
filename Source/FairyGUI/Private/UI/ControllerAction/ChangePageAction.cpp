#include "UI/ControllerAction/ChangePageAction.h"
#include "UI/GController.h"
#include "Utils/ByteBuffer.h"
#include "UI/GComponent.h"

void FChangePageAction::Setup(FByteBuffer* Buffer)
{
    FControllerAction::Setup(Buffer);

    ObjectID = Buffer->ReadS();
    ControllerName = Buffer->ReadS();
    TargetPage = Buffer->ReadS();
}

void FChangePageAction::Enter(UGController* Controller)
{
    if (ControllerName.IsEmpty())
        return;

    UGComponent* gcom;
    if (!ObjectID.IsEmpty())
        gcom = Cast<UGComponent>(Cast<UGComponent>(Controller->GetOuter())->GetChildByID(ObjectID));
    else
        gcom = Cast<UGComponent>(Controller->GetOuter());
    if (gcom != nullptr)
    {
        UGController* cc = gcom->GetController(ControllerName);
        if (cc != nullptr && cc != Controller && !cc->bChanging)
        {
            if (TargetPage.Compare("~1") == 0)
            {
                if (Controller->GetSelectedIndex() < cc->GetPageCount())
                    cc->SetSelectedIndex(Controller->GetSelectedIndex());
            }
            else if (TargetPage.Compare("~2") == 0)
                cc->SetSelectedPage(Controller->GetSelectedPage());
            else
                cc->SetSelectedPageID(TargetPage);
        }
    }
}

void FChangePageAction::Leave(UGController* Controller)
{
}