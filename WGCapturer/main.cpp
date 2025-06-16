#include "WGCapturer.h"

#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>

#include <iostream>


#include <wil/coroutine.h>
#include <wil/resource.h>

#include "util.h"

using namespace winrt;
namespace winrt
{
    using namespace Windows;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Graphics;
    using namespace Windows::Graphics::Capture;
    using namespace Windows::Graphics::DirectX;
    using namespace Windows::Graphics::DirectX::Direct3D11;
    using namespace Windows::System;
}

int main() {
    init_apartment();
    std::cout << "INIT oK" << std::endl;

    HWND hwnd = FindWindowA(nullptr, "MapleStory");
    if (!hwnd) {
        printf("MapleStory not found\n");
        return -1;
    }

    std::cout << "Found oK" << std::endl;
    WgcFrameGrabber grab(hwnd);
    auto frame = grab.CaptureFrame();
    auto mat = grab.frameToCv(frame);
    cv::imwrite("my.png", mat);

    //GraphicsCaptureItem item = CreateCaptureItemForWindow(hwnd);
    //auto d3dDevice = CreateD3D11Device();
    //auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    //auto m_device = CreateDirect3DDevice(dxgiDevice.get());


    //std::cout << "Device oK" << std::endl;
    //// ����첽��������Э�̵ȴ��������
    //auto RunAsync = [&]() -> wil::task<void> {
    //    std::cout << "texture start" << std::endl;
    //    auto texture = co_await CaptureSnapshot::TakeAsync(m_device, item);
    //    std::cout << "texture oK" << std::endl;
    //    SaveTextureToPNG(d3dDevice.get(), texture.get(), "save.png");
    //    std::cout << "save oK" << std::endl;
    //    // ʹ�� texture ...
    //    co_return;
    //    };

    //RunAsync().get(); // ����ͬ���ȴ��첽�������

    
    std::cout << "finish oK" << std::endl;


    return 0;
}
