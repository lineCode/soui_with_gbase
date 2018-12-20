
#ifndef BASE_APPLICATION_H_
#define BASE_APPLICATION_H_

#include <souistd.h>
#include <SApp.h>

#include <base/memory/scoped_ptr.h>
#include <base/message_loop/message_loop.h>

namespace SOUI {

class XApplication : public SOUI::TObjRefImpl<IMsgLoopFactory> {
public:
    class Delegate {
    public:
        virtual ~Delegate() { }

        virtual void PreInitialize();
        virtual void PostInitialize(SApplication *theApp);

        virtual CAutoRefPtr<SOUI::IRenderFactory> CreateRenderFactory();


        virtual LPCTSTR GetHostClassName();
        virtual ISystemObjectRegister *GetSysObjRegister();

        virtual void RegisterResProvider(SApplication *theApp);
        virtual void RegisterSystemResProvider(SApplication *theApp);
        virtual void RegisterExtendSysObjRegister(SApplication *theApp);

        virtual void SetTranslator(SApplication *theApp);  
        
        virtual HWND GetMainHwnd() = 0;
    };

    XApplication(base::MessageLoopForUI* main_loop, Delegate* delegate);

    // 初始化SOUI相关配置
    void Initialize(HINSTANCE hInstance, BOOL bEnableHighDpiSupport = FALSE);
    void Uninitialize();

    // 消息循环
    int RunLoop();

    virtual SMessageLoop * CreateMsgLoop() override;
    virtual void DestoryMsgLoop(SMessageLoop * pMsgLoop) override;

private:
    SApplication *theApp_;
    Delegate* delegate_;
    base::MessageLoopForUI* main_loop_;
};

}
#endif // BASE_APPLICATION_H_