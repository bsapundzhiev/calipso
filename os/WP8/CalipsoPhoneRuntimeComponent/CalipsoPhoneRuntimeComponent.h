#pragma once

namespace CalipsoPhoneRuntimeComponent {

public ref class WindowsPhoneRuntimeComponent sealed {
public:
    WindowsPhoneRuntimeComponent();
public:
    int Start();
    int Stop();

private:
    bool isStarted;
    Windows::Foundation::IAsyncAction^ m_action;
    void cpoWorkerThread(Windows::Foundation::IAsyncAction^ operation);

};
}