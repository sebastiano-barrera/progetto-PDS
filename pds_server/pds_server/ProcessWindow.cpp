#include "stdafx.h"
#include "ProcessWindow.h"
#include "global.h"
#include "keyconv_vk.h"
#include <Psapi.h>
#include <iostream>
#include <string>
#include <codecvt>

INPUT PressKey(int key);
std::string toString(std::wstring wstring);

ProcessWindow::ProcessWindow(HWND hWnd) :
	window_(hWnd)
{
	if (hWnd != (HWND)MAXUINT64) { //if it's equal the focus is on the desktop
		WCHAR wbuf[1024];
		
		if (GetWindowText(hWnd, wbuf, 256) > 0) { //getting window titlte
			title_ = std::wstring(wbuf);
			//std::wcout << "GetWindowText: " << title_ << std::endl;
		}
		
		DWORD proc_id;
		GetWindowThreadProcessId(window_, &proc_id);
		process_ = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, proc_id);
		DWORD wbuf_size = sizeof(wbuf);
		if (QueryFullProcessImageNameW(process_, 0, wbuf, &wbuf_size) > 0) { //getting executable file name
			moduleFileName_ = std::wstring(wbuf);
			//std::wcout << "QueryFullProcessImageNameW: " << moduleFileName_ << std::endl;
		}
		
		icon_ = (HICON) SendMessage(window_, WM_GETICON, ICON_SMALL2, 0);
		if (icon_ == 0)
			icon_ = (HICON) SendMessage(window_, WM_GETICON, ICON_SMALL, 0);
		if (icon_ == 0)
			icon_ = (HICON) SendMessage(window_, WM_GETICON, ICON_BIG, 0);
		if (icon_ == 0)
			icon_ = (HICON) GetClassLongPtr(window_, GCL_HICON);
		if (icon_ == 0)
			icon_ = (HICON) GetClassLongPtr(window_, GCL_HICONSM);

	}
	else {
		title_ = L"Desktop";
	}
}

//prints ProcessWindow details
void ProcessWindow::windowInfo() const
{
	std::wcout << "Window: Title:" << title_ << " (" << moduleFileName_ << ", icon: " << icon_ << ")" << std::endl;
}

std::unique_ptr<msgs::Icon> ProcessWindow::encodeIcon() const
{
	// NOTE: The following code does not make any effort to support
	// paletted (aka color-indexed) formats, since we don't make use
	// of them whatsoever, and it makes the code less readable and more
	// complex.

	ICONINFO icon_info;

	if (GetIconInfo(icon_, &icon_info) == FALSE)
		return nullptr;
	
	BITMAP bmp;
	if (!icon_info.hbmColor) {
		std::wcerr << "warning: required icon is black/white (not yet implemented)";
		return nullptr;
	}
	
	if (GetObject(icon_info.hbmColor, sizeof(bmp), &bmp) <= 0)
		return nullptr;

	// Allocate memory for the header (should also make space for the color table,
	// but we're not using it, so no need for that)
	BITMAPV5HEADER *hdr = (BITMAPV5HEADER*) std::malloc(sizeof(*hdr));
	hdr->bV5Size = sizeof(BITMAPV5HEADER);
	hdr->bV5Width = bmp.bmWidth;
	hdr->bV5Height = bmp.bmHeight;
	hdr->bV5Planes = 1;
	// 4 bytes per pixel: (hi) ARGB (lo)
	hdr->bV5BitCount = 32;
	hdr->bV5Compression = BI_BITFIELDS;
	hdr->bV5RedMask   = 0x00FF0000;
	hdr->bV5GreenMask = 0x0000FF00;
	hdr->bV5BlueMask  = 0x000000FF;
	hdr->bV5AlphaMask = 0xFF000000;
	// will compute this one later
	hdr->bV5SizeImage = 0;
	// this means: don't use/store a palette
	hdr->bV5XPelsPerMeter = 0;  
	hdr->bV5YPelsPerMeter = 0;
	hdr->bV5ClrUsed = 0;
	hdr->bV5ClrImportant = 0;

	HDC hdc = GetDC(NULL);

	// Make the device driver calculate the image data size (biSizeImage)
	GetDIBits(hdc, icon_info.hbmColor, 0L, bmp.bmHeight,
		NULL, (BITMAPINFO*) hdr, DIB_RGB_COLORS);

	const size_t scanline_bytes = (((hdr->bV5Width * hdr->bV5BitCount) + 31) & ~31) / 8;
	if (hdr->bV5SizeImage == 0) {
		// Well, that didn't work out. Calculate bV5SizeImage ourselves.
		// The form ((x + n) & ~n) is a trick to round x up to a multiple of n+1.
		// In this case, a multiple of 32 (DWORD-aligned)
		hdr->bV5SizeImage = scanline_bytes * hdr->bV5Height;
	}

	// Make space for the image pixels data
	void *new_hdr = std::realloc(hdr, sizeof(BITMAPV5HEADER) + hdr->bV5SizeImage);
	if (!new_hdr) {
		std::free(hdr);
		ReleaseDC(NULL, hdc);
		return nullptr;
	}
	hdr = (BITMAPV5HEADER*)new_hdr;

	void *pixels = ((uint8_t*) hdr) + sizeof(BITMAPV5HEADER);
	BOOL got_bits = GetDIBits(hdc, icon_info.hbmColor, 
		0L, bmp.bmHeight,
		(LPBYTE) pixels,
		(BITMAPINFO*)hdr,
		DIB_RGB_COLORS);

	ReleaseDC(NULL, hdc);
	
	if (got_bits == FALSE) {
		// Well, damn.
		std::free(hdr);
		return nullptr;
	}

	// Got the bitmap data ... upside down and with channels inverted (BGR).
	// The following adapts the data we have to the format we want (24 bits per pixel, uncompressed RGB).
	// This code won't work after changing either format.

	// Invert order of channels (BGRA -> ARGB)
	// Turn image upside down
	typedef unsigned char color_t[4];
	for (int y = 0; y < hdr->bV5Height/2; y++) {
		color_t *scanline_top = (color_t*)((char*)pixels + y * scanline_bytes);
		color_t *scanline_bot = (color_t*)((char*)pixels + (hdr->bV5Height - y - 1) * scanline_bytes);
		for (int x = 0; x < hdr->bV5Width; x++)
			std::swap(scanline_top[x], scanline_bot[x]);
	}

	auto icon = std::make_unique<msgs::Icon>();
	icon->set_width(hdr->bV5Width);
	icon->set_height(hdr->bV5Height);
	icon->set_pixels(pixels, hdr->bV5SizeImage);
	
	std::free(hdr);  // also frees pixels
	return icon;
}

HWND ProcessWindow::handle() const
{
	return window_;
}

std::string ProcessWindow::title() const
{
	return toString(this->title_);
}

std::string ProcessWindow::moduleFileName() const {
	return toString(this->moduleFileName_);
}

//wstring to string converter
std::string toString(std::wstring wstring) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conversion;
	return conversion.to_bytes(wstring);
}

bool ProcessWindow::sendKeystroke(msgs::KeystrokeRequest req)
{
	int num_mods = 0;
	INPUT ip[5];
	
	if (req.ctrl()) {
		ip[num_mods++] = PressKey(VK_CONTROL);
	}
	if (req.alt()) {
		ip[num_mods++] = PressKey(VK_MENU);
	}
	if (req.shift()) {
		ip[num_mods++] = PressKey(VK_SHIFT);
	}
	if (req.meta()) {
		ip[num_mods++] = PressKey(VK_LWIN);
	}
	
	int virtual_key = keyconv::vk_of_proto(req.key());
	if (virtual_key == -1)  // invalid key
		return false;

	ip[num_mods++] = PressKey(virtual_key);
	if (SendInput(num_mods, ip, sizeof(INPUT))) {
		for (int i = 0; i < num_mods; i++) {
			ip[i].ki.dwFlags = KEYEVENTF_KEYUP;
		}

		if (SendInput(num_mods, ip, sizeof(INPUT))) {
			return true;
		}
	}

	std::cerr << "unable to send input to the selected window" << std::endl;
	return false;
}

INPUT PressKey(int key) {
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = key;
	input.ki.time = 0;
	input.ki.wScan = MapVirtualKey(key, MAPVK_VK_TO_VSC); 
	input.ki.dwFlags = 0;
	input.ki.dwExtraInfo = 0;
	return input;
}
