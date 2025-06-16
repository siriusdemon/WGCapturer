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



cv::Mat WGCapturer::frameToCv(winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame& frame) {
    auto texture = GetDXGIInterfaceFromSurface<ID3D11Texture2D>(frame.Surface());
    auto result = CopyD3DTexture(d3dDevice.get(), texture.get(), true);

    return Texture2Mat(d3dDevice.get(), result.get());
}

WGCapturer::WGCapturer(HWND hwnd) {
    winrt::init_apartment();
    this->hwnd = hwnd;
    this->m_item = CreateCaptureItemForWindow(hwnd);
    this->d3dDevice = CreateD3D11Device();
    auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    this->m_device = CreateDirect3DDevice(dxgiDevice.get());
    this->m_format = winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized;

    m_framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
        m_device, m_format, 2, m_item.Size());

    m_session = m_framePool.CreateCaptureSession(m_item);
    m_session.StartCapture();
    this->crop = false;
}

winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame WGCapturer::CaptureFrame() {
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame frame{ nullptr };
    for (int i = 0; i < 10; i++) {
        frame = m_framePool.TryGetNextFrame();
        if (frame) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return frame;
}


void WGCapturer::setTargetRegion(int left, int top, int width, int height) {
    this->crop = true;
    this->Left = left;
    this->Top = top;
    this->Width = width;
    this->Height = height;

}

std::vector<cv::Mat> WGCapturer::capture(const std::vector<std::tuple<int, int, int, int>>& regions) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto frame = this->CaptureFrame();
    auto mat = this->frameToCv(frame);
    frame = nullptr; // drop  it

    // crop before return any region to align with gdi method's region
    if (crop) {
        cv::Rect roi(this->Left, this->Top, this->Width, this->Height);
        mat = mat(roi);
    }

    int width = mat.cols;
    int height = mat.rows;
    

    std::vector<cv::Mat> results;
    // convert region data
    if (!regions.empty()) {
        for (auto& region : regions) {
            auto& [left, top, right, bottom] = region;
            if (left < 0 || top < 0 || right > width || bottom > height || right <= left || bottom <= top) {
                std::cout << "Region out of bounds: [" << left << ", " << top << ", " << right << ", " << bottom << "]" << std::endl;
                results.push_back(cv::Mat());
                return results;
            }
            cv::Rect roi(left, top, right - left, bottom - top);
            cv::Mat resultMat = mat(roi);
            results.push_back(std::move(resultMat));
        }
    }
    // convert all data
    results.push_back(std::move(mat));
    return results;
}
