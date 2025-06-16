#include "pch.h"
#include "WGCapturer.h"
#include "util.h"

#include <iostream>

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


winrt::com_ptr<ID3D11Texture2D> CopyD3DTexture(
    ID3D11Device* device,
    ID3D11Texture2D* source,
    bool makeStaging)
{
    winrt::com_ptr<ID3D11Texture2D> result;

    if (!makeStaging)
    {
        result.copy_from(source);
        return result;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    source->GetDesc(&desc);

    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.MiscFlags = 0;

    winrt::check_hresult(device->CreateTexture2D(&stagingDesc, nullptr, result.put()));

    winrt::com_ptr<ID3D11DeviceContext> context;
    device->GetImmediateContext(context.put());

    context->CopyResource(result.get(), source);

    return result;
}


/*
wil::task<winrt::com_ptr<ID3D11Texture2D>>
CaptureSnapshot::TakeAsync(winrt::IDirect3DDevice const& device, winrt::GraphicsCaptureItem const& item, winrt::DirectXPixelFormat const& pixelFormat)
{
    // Grab the apartment context so we can return to it.
    std::cout << "start capture" << std::endl;
    winrt::apartment_context context;

    std::cout << "get dxgi interface " << std::endl;
    auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(device);
    winrt::com_ptr<ID3D11DeviceContext> d3dContext;
    std::cout << "get immediate context " << std::endl;
    d3dDevice->GetImmediateContext(d3dContext.put());

    // Creating our frame pool with CreateFreeThreaded means that we 
    // will be called back from the frame pool's internal worker thread
    // instead of the thread we are currently on. It also disables the
    // DispatcherQueue requirement.
    std::cout << "frame pool" << std::endl;
    auto framePool = winrt::Direct3D11CaptureFramePool::CreateFreeThreaded(
        device,
        pixelFormat,
        1,
        item.Size());
    std::cout << "capturesession" << std::endl;
    auto session = framePool.CreateCaptureSession(item);

    wil::shared_event captureEvent(wil::EventOptions::ManualReset);
    winrt::Direct3D11CaptureFrame frame{ nullptr };
    framePool.FrameArrived([&frame, captureEvent](auto& framePool, auto&)
        {
            frame = framePool.TryGetNextFrame();

            // Complete the operation
            captureEvent.SetEvent();
        });

    std::cout << "start catpure" << std::endl;
    session.StartCapture();
    co_await winrt::resume_on_signal(captureEvent.get());
    co_await context;

    // End the capture
    session.Close();
    framePool.Close();
    std::cout << "finish catpure" << std::endl;

    auto texture = GetDXGIInterfaceFromSurface<ID3D11Texture2D>(frame.Surface());
    auto result = CopyD3DTexture(d3dDevice.get(), texture.get(), true);

    co_return result;
}
*/

cv::Mat WgcFrameGrabber::frameToCv(winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame& frame) {
    auto texture = GetDXGIInterfaceFromSurface<ID3D11Texture2D>(frame.Surface());
    auto result = CopyD3DTexture(d3dDevice.get(), texture.get(), true);

    return Texture2Mat(d3dDevice.get(), result.get());
}

WgcFrameGrabber::WgcFrameGrabber(HWND hwnd) {
    this->hwnd = hwnd;
    this->m_item = CreateCaptureItemForWindow(hwnd);
    this->d3dDevice = CreateD3D11Device();
    auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    this->m_device = CreateDirect3DDevice(dxgiDevice.get());
    this->m_format = winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized;

    m_framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
        m_device, m_format, 1, m_item.Size());

    m_session = m_framePool.CreateCaptureSession(m_item);
    m_session.StartCapture();
    std::cout << "Start catpure\n";
}

winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame WgcFrameGrabber::CaptureFrame() {
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame frame{ nullptr };
    for (int i = 0; i < 10; i++) {
        frame = m_framePool.TryGetNextFrame();
        if (frame) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return frame;
}
