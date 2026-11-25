// Structural regression guards only: NOT evidence of real desktop tray/input success.
const fs = require('node:fs');
const assert = require('node:assert/strict');
const path = require('node:path');
const source = fs.readFileSync(path.join(__dirname, '../src/main-updated.cpp'), 'utf8').replace(/\r\n/g, '\n');
const hooks = source.slice(source.indexOf('int key_hook()'), source.indexOf('#define IDR_PAUSE'));
const windowProc = source.slice(source.indexOf('LRESULT CALLBACK WndProc('), source.indexOf('int SetIco()'));
const tray = source.slice(source.indexOf('int SetIco()'), source.indexOf('int main()'));
assert(hooks.indexOf('SetEvent(keyboardReady)') > hooks.indexOf('hooksInstalled.store('), 'ready must follow hook install outcome');
assert(hooks.includes('keyboardHookError.store(error)'), 'keyboard install errors must be retained');
assert(source.includes('mouseHookError.store(error)'), 'mouse install errors must be retained');
assert(tray.includes('WNDCLASSW wndclass{}'), 'window class must be zero initialized');
assert(tray.includes('wndclass.hInstance = GetModuleHandleW(NULL)'), 'registration must use executable module');
assert(windowProc.includes('default:\n\t\treturn DefWindowProcW('), 'only unhandled messages should reach DefWindowProc');
assert(/return 0;\n}\s*(?:\/\/[^\n]*\s*)?$/.test(windowProc), 'handled messages must return zero');
assert(windowProc.includes('taskbarCreatedMessage && message == taskbarCreatedMessage'), 'Explorer restart must re-register tray');
assert(windowProc.includes('KillTimer(hwnd, 1)'), 'destroy must cancel tray retries');
assert(tray.includes('(result = GetMessageW(&msg, NULL, 0, 0)) > 0'), 'message-loop failure must not dispatch stale messages');
console.log('PASS: tray/global-hook structural regression guards (not desktop verification)');
