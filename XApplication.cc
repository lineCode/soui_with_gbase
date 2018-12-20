#include "XApplication.h"

#include <ShellScalingAPI.h>
#include <shellapi.h>

#include <render-gdi/render-gdi.h>
#include <imgdecoder-gdip/imgdecoder-gdip.h>
#include <translator/translator.h>

#include <base/run_loop.h>
#include <base/message_loop/message_pump_dispatcher.h>
#include <base/win/windows_version.h>

namespace {

// Win8.1 supports monitor-specific DPI scaling.
bool SetProcessDpiAwarenessWrapper(PROCESS_DPI_AWARENESS value) {
    typedef HRESULT(WINAPI *SetProcessDpiAwarenessPtr)(PROCESS_DPI_AWARENESS);
    SetProcessDpiAwarenessPtr set_process_dpi_awareness_func =
        reinterpret_cast<SetProcessDpiAwarenessPtr>(
            GetProcAddress(GetModuleHandleA("user32.dll"),
                           "SetProcessDpiAwarenessInternal"));
    if (set_process_dpi_awareness_func) {
        HRESULT hr = set_process_dpi_awareness_func(value);
        if (SUCCEEDED(hr)) {
            VLOG(1) << "SetProcessDpiAwareness succeeded.";
            return true;
        }
        else if (hr == E_ACCESSDENIED) {
            LOG(ERROR) << "Access denied error from SetProcessDpiAwareness. "
                "Function called twice, or manifest was used.";
        }
    }
    return false;
}

// This function works for Windows Vista through Win8. Win8.1 must use
// SetProcessDpiAwareness[Wrapper].
BOOL SetProcessDPIAwareWrapper() {
    typedef BOOL(WINAPI *SetProcessDPIAwarePtr)(VOID);
    SetProcessDPIAwarePtr set_process_dpi_aware_func =
        reinterpret_cast<SetProcessDPIAwarePtr>(
            GetProcAddress(GetModuleHandleA("user32.dll"),
                           "SetProcessDPIAware"));
    return set_process_dpi_aware_func &&
        set_process_dpi_aware_func();
}

void EnableHighDPISupport() {
    // Enable per-monitor DPI for Win10 or above instead of Win8.1 since Win8.1
    // does not have EnableChildWindowDpiMessage, necessary for correct non-client
    // area scaling across monitors.
    bool allowed_platform = base::win::GetVersion() >= base::win::VERSION_WIN10;
    PROCESS_DPI_AWARENESS process_dpi_awareness = PROCESS_PER_MONITOR_DPI_AWARE;
    if (!SetProcessDpiAwarenessWrapper(process_dpi_awareness)) {
        SetProcessDPIAwareWrapper();
    }
}

}
namespace SOUI {

class XMessageLoop :
    public SMessageLoop,
    public base::MessagePumpDispatcher {
public:
    XMessageLoop(base::MessageLoopForUI* main_loop)
        : main_loop_(main_loop) {
    }

    ~XMessageLoop() {

    }

    virtual int Run() override {
        base::RunLoop runloop(this);
        runloop.Run();
        return 0;
    }

    virtual void Quit() override {
        base::MessageLoop::current()->QuitNow();
    }

    virtual BOOL IsRunning() const override {
        return base::MessageLoop::current()->is_running() ? TRUE : FALSE;
    }

    virtual uint32_t Dispatch(const base::NativeEvent& event) override {
        return PreTranslateMessage(const_cast<MSG*>(&event)) ?
            base::MessagePumpDispatcher::POST_DISPATCH_NONE :
            base::MessagePumpDispatcher::POST_DISPATCH_PERFORM_DEFAULT;
    }

private:
    base::MessageLoopForUI* main_loop_;
};


//////////////////////////////////////////////////////////////////////////
void XApplication::Delegate::PreInitialize() {

}

void XApplication::Delegate::PostInitialize(SApplication *theApp) {

}

CAutoRefPtr<IRenderFactory> XApplication::Delegate::CreateRenderFactory() {
    CAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory;
    CAutoRefPtr<IRenderFactory> pRenderFactory;
    RENDER_GDI::SCreateInstance((IObjRef**)&pRenderFactory);
    IMGDECODOR_GDIP::SCreateInstance((IObjRef**)&pImgDecoderFactory);
    pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
    return pRenderFactory;
}

LPCTSTR XApplication::Delegate::GetHostClassName() {
    return nullptr;
}

ISystemObjectRegister * XApplication::Delegate::GetSysObjRegister() {
    return nullptr;
}

void XApplication::Delegate::RegisterResProvider(SApplication *theApp) {
    CAutoRefPtr<IResProvider> pResProvider;
    CreateResProvider(RES_PE, (IObjRef**)&pResProvider);

    pResProvider->Init((WPARAM)theApp->GetInstance(), 0);
    theApp->AddResProvider(pResProvider);
}

void XApplication::Delegate::RegisterSystemResProvider(SApplication *theApp) {
    CAutoRefPtr<IResProvider> pResProvider;
    CreateResProvider(RES_PE, (IObjRef**)&pResProvider);

    pResProvider->Init((WPARAM)theApp->GetInstance(), 0);
    theApp->LoadSystemNamedResource(pResProvider);
}

void XApplication::Delegate::RegisterExtendSysObjRegister(SApplication *theApp) {

}

void XApplication::Delegate::SetTranslator(SApplication *theApp) {

}

//////////////////////////////////////////////////////////////////////////
XApplication::XApplication(base::MessageLoopForUI* main_loop, Delegate* delegate) : 
    main_loop_(main_loop),
    delegate_(delegate) {

}

void XApplication::Initialize(HINSTANCE hInstance, BOOL bEnableHighDpiSupport) {
    if (bEnableHighDpiSupport) {
        EnableHighDPISupport();
    }

    delegate_->PreInitialize();

    theApp_ = new SApplication(delegate_->CreateRenderFactory(),
                                     hInstance,
                                     delegate_->GetHostClassName(),
                                     delegate_->GetSysObjRegister());

    theApp_->SetMsgLoopFactory(this);

    delegate_->RegisterExtendSysObjRegister(theApp_);

    delegate_->RegisterResProvider(theApp_);
    delegate_->RegisterSystemResProvider(theApp_);
    delegate_->SetTranslator(theApp_);    

    delegate_->PostInitialize(theApp_);
}

void XApplication::Uninitialize() {
    main_loop_->QuitNow();
    delete theApp_;
}

int XApplication::RunLoop() {
    return theApp_->Run(delegate_->GetMainHwnd());
}

SMessageLoop* XApplication::CreateMsgLoop() {
    return new XMessageLoop(main_loop_);
}

void XApplication::DestoryMsgLoop(SMessageLoop * pMsgLoop) {
    delete pMsgLoop;
}

}
