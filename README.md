# soui_with_gbase

### 使用方法
```
#include <base/at_exit.h>
#include <base/command_line.h>
#include <base/win/message_window.h>

namespace SOUI {
class XApplicationDelegateImpl : public XApplication::Delegate {
public:
    ApplicationDelegateImpl();
    ~ApplicationDelegateImpl();

    virtual void PreInitialize();

    virtual void PostInitialize(SOUI::SApplication *theApp);

    virtual HWND GetMainHwnd() override;

    virtual LPCTSTR GetHostClassName() override;

    virtual void RegisterExtendSysObjRegister(SApplication *theApp) override;

private:
    scoped_ptr<ui::ScopedOleInitializer> ole_initializer_;

    std::wstring               cls_name_;
    scoped_ptr<SOUI::MainWindow> main_;
};



class MainWindow : public SHostWnd {
public:
  // ....
};

// ...
XApplicationDelegateImpl::PostInitialize(SOUI::SApplication *theApp) {
    main_.reset(new SOUI::MainWindow);
    main_->Create(NULL, WS_POPUPWINDOW | WS_CLIPCHILDREN, 0, 0, 0, 0, 0);
    main_->SendMessage(WM_INITDIALOG);
    main_->ShowWindow(SW_NORMAL);
}

HWND XApplicationDelegateImpl::GetMainHwnd() {
  return main_.m_hWnd;
}

// ...
// MainWindow
//
}



int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {

    base::AtExitManager AtExitManager;
    base::CommandLine::Init(0, nullptr);
    base::MessageLoopForUI main_loop;
    
    SOUI::XApplicationDelegateImpl application_delegate;
    SOUI::XApplication application(&main_loop, &application_delegate);
    application.Initialize(hInstance, TRUE);
    application.RunLoop();
    
    return 0;
}
```        
