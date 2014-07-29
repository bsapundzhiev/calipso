// CalipsoPhoneRuntimeComponent.cpp
#include "pch.h"
#include "CalipsoPhoneRuntimeComponent.h"
#include "CalipsoDLL.h"

#include "ppltasks.h"

using namespace CalipsoPhoneRuntimeComponent;
using namespace Platform;
//using namespace ::Windows::Threading;
using namespace Windows::System;
using namespace Windows::System::Threading;
using namespace Windows::Foundation;
using namespace concurrency;

WindowsPhoneRuntimeComponent::WindowsPhoneRuntimeComponent()
{
	isStarted = false;
}

int WindowsPhoneRuntimeComponent::Start()
{
	if(isStarted == false) {
		calipso_main();
		//auto asyncAction = ThreadPool::RunAsync(ref new WorkItemHandler(this, &WindowsPhoneRuntimeComponent::cpoWorkerThread));
		
		//auto asyncAction = ThreadPool::RunAsync(ref new WorkItemHandler([] (IAsyncAction^ operation) {
		//		calipso_start_thread();
		//}, Platform::CallbackContext::Same));

	
		auto asyncAction = create_async([](cancellation_token ct){
			calipso_start_thread(/*ct.is_canceled()*/);
		});

		if(asyncAction) {
			m_action = asyncAction;
			isStarted = true;
		}
	}
	
	return isStarted;
}
		
int  WindowsPhoneRuntimeComponent::Stop()
{
	if(isStarted == true) {
		m_action->Close();
		isStarted = false;
	}

	return isStarted;
}

void WindowsPhoneRuntimeComponent::cpoWorkerThread(Windows::Foundation::IAsyncAction^ operation)
{
	calipso_start_thread();
}