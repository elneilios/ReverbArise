#include "NativeWavDrag.h"
#include <UI/WorkspaceTheme.h>

#if JUCE_WINDOWS
#include <windows.h>
#include <shlobj.h>
#include <shlguid.h>
#include <wrl/client.h>

namespace m3::ui::controls
{
namespace
{
using Microsoft::WRL::ComPtr;
constexpr float wavBadgeLabelHeight = 8.5f; // Scoped to the approved native cursor artwork.
juce::Image drawDragCursor()
{
    juce::Image image(juce::Image::ARGB, 88, 72, true);
    juce::Graphics g(image);
    g.addTransform(juce::AffineTransform::scale(2.0f));

    juce::Path hand;
    hand.startNewSubPath(5, 12);
    hand.lineTo(5, 7);
    hand.quadraticTo(5, 4, 8, 5);
    hand.lineTo(9, 7);
    hand.lineTo(9, 4);
    hand.quadraticTo(11, 1, 13, 4);
    hand.lineTo(13, 6);
    hand.quadraticTo(15, 3, 17, 6);
    hand.lineTo(17, 8);
    hand.quadraticTo(20, 6, 21, 9);
    hand.lineTo(21, 15);
    hand.quadraticTo(21, 19, 18, 22);
    hand.lineTo(18, 24);
    hand.lineTo(9, 24);
    hand.lineTo(9, 22);
    hand.lineTo(3, 16);
    hand.quadraticTo(1, 13, 3, 12);
    hand.quadraticTo(4, 11, 5, 12);
    hand.closeSubPath();
    g.setColour(text);
    g.fillPath(hand);
    g.setColour(background);
    g.strokePath(hand, juce::PathStrokeType(1.4f, juce::PathStrokeType::curved));
    for (const auto x : {9.0f, 13.0f, 17.0f}) g.drawLine(x, 7, x, 12, 1.0f);

    juce::Path badge;
    badge.startNewSubPath(20, 14);
    badge.lineTo(34, 14);
    badge.lineTo(41, 21);
    badge.lineTo(41, 33);
    badge.lineTo(20, 33);
    badge.closeSubPath();
    g.fillPath(badge);
    g.setColour(aqua);
    g.strokePath(badge, juce::PathStrokeType(1.25f));
    g.drawLine(34, 14, 34, 21, 1.0f);
    g.drawLine(34, 21, 41, 21, 1.0f);
    g.fillRect(21, 24, 19, 8);
    g.setColour(background);
    g.setFont(interFont(wavBadgeLabelHeight, InterFace::bold));
    g.drawText("WAV", 21, 23, 19, 10, juce::Justification::centred);
    return image;
}

HCURSOR createWavCursor(UINT dpi)
{
    const auto scale = static_cast<float>(dpi) / 96.0f;
    const auto image = drawDragCursor().rescaled(juce::roundToInt(44 * scale), juce::roundToInt(36 * scale));
    const auto width = image.getWidth();
    const auto height = image.getHeight();
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    void* bits = nullptr;
    const auto colour = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (colour == nullptr) return nullptr;

    auto* pixels = static_cast<uint32_t*>(bits);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            pixels[y * width + x] = image.getPixelAt(x, y).getARGB();

    const std::vector<uint8_t> maskBits(static_cast<size_t>(((width + 15) / 16) * 2 * height), 0);
    const auto mask = CreateBitmap(width, height, 1, 1, maskBits.data());
    ICONINFO icon{};
    icon.xHotspot = static_cast<DWORD>(juce::roundToInt(12 * scale));
    icon.yHotspot = icon.xHotspot;
    icon.hbmMask = mask;
    icon.hbmColor = colour;
    const auto cursor = mask != nullptr ? CreateIconIndirect(&icon) : nullptr;
    if (mask != nullptr) DeleteObject(mask);
    DeleteObject(colour);
    return cursor;
}

class DropSource final : public IDropSource
{
public:
    DropSource(HCURSOR wav, const std::function<bool()>& current) : wavCursor(wav), canContinue(current) {}
    ~DropSource() { if (wavCursor != nullptr) DestroyCursor(wavCursor); }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** result) override
    {
        if (result == nullptr) return E_POINTER;
        *result = nullptr;
        if (id != IID_IUnknown && id != IID_IDropSource) return E_NOINTERFACE;
        *result = static_cast<IDropSource*>(this);
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++references; }
    ULONG STDMETHODCALLTYPE Release() override
    {
        const auto remaining = --references;
        if (remaining == 0) delete this;
        return remaining;
    }

    HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL escape, DWORD buttons) override
    {
        if (escape || (buttons & MK_RBUTTON) != 0 || !canContinue()) return DRAGDROP_S_CANCEL;
        return (buttons & MK_LBUTTON) == 0 ? DRAGDROP_S_DROP : S_OK;
    }

    HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD effect) override
    {
        if ((effect & DROPEFFECT_COPY) == 0)
        {
            SetCursor(LoadCursor(nullptr, IDC_NO));
            return S_OK;
        }
        if (wavCursor == nullptr) return DRAGDROP_S_USEDEFAULTCURSORS;
        SetCursor(wavCursor);
        return S_OK;
    }

private:
    ULONG references = 1;
    HCURSOR wavCursor;
    const std::function<bool()> canContinue;
};

ComPtr<IDataObject> createFileDataObject(const juce::File& file)
{
    ComPtr<IShellItem> item;
    ComPtr<IDataObject> data;
    if (FAILED(SHCreateItemFromParsingName(file.getFullPathName().toWideCharPointer(), nullptr,
                                           IID_PPV_ARGS(item.GetAddressOf())))
        || FAILED(item->BindToHandler(nullptr, BHID_DataObject, IID_PPV_ARGS(data.GetAddressOf()))))
        return {};

    FORMATETC format{CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    return data->QueryGetData(&format) == S_OK ? data : ComPtr<IDataObject>{};
}

struct OleScope
{
    HRESULT result = OleInitialize(nullptr);
    ~OleScope() { if (SUCCEEDED(result)) OleUninitialize(); }
};
}

WavDragResult dragReadyWav(const juce::File& file, juce::Component& source,
                          const std::function<bool()>& canStart, const std::function<bool()>& canContinue)
{
    if (!file.existsAsFile() || source.getPeer() == nullptr) return WavDragResult::unavailable;
    if (!canStart()) return WavDragResult::cancelled;
    const auto window = static_cast<HWND>(source.getPeer()->getNativeHandle());
    const auto dpi = GetDpiForWindow(window);
    OleScope ole;
    if (FAILED(ole.result)) return WavDragResult::unavailable;

    const auto data = createFileDataObject(file);
    if (data == nullptr) return WavDragResult::unavailable;

    const auto cursor = createWavCursor(dpi == 0 ? 96 : dpi);
    ComPtr<IDropSource> dropSource;
    dropSource.Attach(new DropSource(cursor, canContinue));
    if (!canStart() || (GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0) return WavDragResult::cancelled;

    DWORD effect = DROPEFFECT_NONE;
    const auto result = DoDragDrop(data.Get(), dropSource.Get(), DROPEFFECT_COPY, &effect);
    SetCursor(LoadCursor(nullptr, IDC_ARROW));
    if (result == DRAGDROP_S_DROP && (effect & DROPEFFECT_COPY) != 0) return WavDragResult::dropped;
    return SUCCEEDED(result) ? WavDragResult::cancelled : WavDragResult::unavailable;
}
}
#else
namespace m3::ui::controls
{
WavDragResult dragReadyWav(const juce::File&, juce::Component&, const std::function<bool()>&, const std::function<bool()>&)
{
    return WavDragResult::unavailable;
}
}
#endif
