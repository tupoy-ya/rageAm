#include "pgBase.h"

#include "TlsManager.h"
#include "datResource.h"

const rage::datResource* rage::pgBase::GetResource()
{
	return TlsManager::GetResource();
}

rage::pgBase::pgBase()
{
	if (qword8 == 0)
		return;

	const datResource* rsc = GetResource();
	if (!rsc)
		return;

	rsc->Fixup(qword8);
}
