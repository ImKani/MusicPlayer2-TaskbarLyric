# MusicPlayer2 Taskbar Lyric IPC API

本文档说明 `MusicPlayer2-TaskbarLyric` 修改版提供的任务栏歌词接口，供 TrafficMonitor 插件或其他本地程序适配。

这个接口不是 HTTP/Web API，而是一个 Windows 本地 IPC 接口：修改版 MusicPlayer2 将当前播放和歌词状态写入命名共享内存，第三方程序只读该共享内存即可获得当前歌词、下一句歌词、翻译、多行并列歌词、曲名和歌手等信息。

## 接口状态

- 接口类型：Windows named file mapping/shared memory
- 提供方：修改版 MusicPlayer2
- 消费方：TrafficMonitor 插件、桌面歌词工具、状态栏工具等本地程序
- 字符编码：UTF-16LE，也就是 Windows `wchar_t`
- 当前协议版本：`3`
- 线程/进程模型：单写多读
- 稳定性：个人修改版接口，字段和版本可能随 fork 演进变化；消费者必须校验 `magic` 和 `version`

## 共享内存

```cpp
constexpr wchar_t MP2_LYRIC_IPC_NAME[] = L"Local\\MusicPlayer2_TaskbarLyric_State";
constexpr DWORD MP2_LYRIC_IPC_MAGIC = 0x324C504D; // MPL2
constexpr DWORD MP2_LYRIC_IPC_VERSION = 3;
```

消费者使用：

```cpp
OpenFileMappingW(FILE_MAP_READ, FALSE, L"Local\\MusicPlayer2_TaskbarLyric_State")
MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, sizeof(Mp2LyricIpcState))
```

如果 `OpenFileMappingW` 失败，通常表示修改版 MusicPlayer2 未运行，或尚未发布过歌词状态。

## 数据结构

消费者应复制以下结构体定义，并保持字段顺序、字段类型和数组长度一致。

```cpp
#include <Windows.h>

constexpr wchar_t MP2_LYRIC_IPC_NAME[] = L"Local\\MusicPlayer2_TaskbarLyric_State";
constexpr DWORD MP2_LYRIC_IPC_MAGIC = 0x324C504D; // MPL2
constexpr DWORD MP2_LYRIC_IPC_VERSION = 3;

struct Mp2LyricIpcState
{
    DWORD magic{};
    DWORD version{};
    DWORD sequence{};
    DWORD process_id{};
    ULONGLONG tick_count{};
    BOOL running{};
    BOOL playing{};
    BOOL has_lyric{};
    int progress{};
    wchar_t title[256]{};
    wchar_t artist[256]{};
    wchar_t current_lyric[512]{};
    wchar_t current_translate[512]{};
    wchar_t next_lyric[512]{};
    wchar_t next_translate[512]{};
    DWORD current_parallel_count{};
    wchar_t current_parallel_1[512]{};
    wchar_t current_parallel_2[512]{};
    wchar_t current_parallel_3[512]{};
    DWORD next_parallel_count{};
    wchar_t next_parallel_1[512]{};
    wchar_t next_parallel_2[512]{};
    wchar_t next_parallel_3[512]{};
};
```

## 字段说明

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `magic` | `DWORD` | 固定为 `MP2_LYRIC_IPC_MAGIC`。消费者必须校验。 |
| `version` | `DWORD` | 协议版本。当前为 `3`。消费者必须校验。 |
| `sequence` | `DWORD` | 每次发布递增。消费者可用它判断状态是否变化。 |
| `process_id` | `DWORD` | 发布状态的 MusicPlayer2 进程 ID。 |
| `tick_count` | `ULONGLONG` | 发布时的 `GetTickCount64()` 值，可用于判断状态新鲜度。 |
| `running` | `BOOL` | `TRUE` 表示 MusicPlayer2 正在运行并发布状态；关闭或清理时会设为 `FALSE`。 |
| `playing` | `BOOL` | `TRUE` 表示当前处于播放状态。暂停时通常为 `FALSE`。 |
| `has_lyric` | `BOOL` | `TRUE` 表示当前字段来自有效歌词；`FALSE` 时可能是歌曲信息或无歌词状态。 |
| `progress` | `int` | 当前歌词行进度，范围通常为 `0` 到 `1000`。`1000` 表示当前歌词已结束。 |
| `title` | `wchar_t[256]` | 当前歌曲标题。 |
| `artist` | `wchar_t[256]` | 当前歌曲歌手。 |
| `current_lyric` | `wchar_t[512]` | 当前歌词主行。 |
| `current_translate` | `wchar_t[512]` | 当前歌词选中的翻译/副行，用于兼容原本只支持一条翻译的显示逻辑。 |
| `next_lyric` | `wchar_t[512]` | 下一句歌词主行。 |
| `next_translate` | `wchar_t[512]` | 下一句歌词选中的翻译/副行。 |
| `current_parallel_count` | `DWORD` | 当前歌词同时间戳并列副行数量，最多 `3`。 |
| `current_parallel_1` | `wchar_t[512]` | 当前歌词第 1 条并列副行。对应同时间戳的第 2 行。 |
| `current_parallel_2` | `wchar_t[512]` | 当前歌词第 2 条并列副行。对应同时间戳的第 3 行。 |
| `current_parallel_3` | `wchar_t[512]` | 当前歌词第 3 条并列副行。对应同时间戳的第 4 行。 |
| `next_parallel_count` | `DWORD` | 下一句歌词同时间戳并列副行数量，最多 `3`。 |
| `next_parallel_1` | `wchar_t[512]` | 下一句歌词第 1 条并列副行。 |
| `next_parallel_2` | `wchar_t[512]` | 下一句歌词第 2 条并列副行。 |
| `next_parallel_3` | `wchar_t[512]` | 下一句歌词第 3 条并列副行。 |

所有字符串都会以 `wcsncpy_s(..., _TRUNCATE)` 写入，因此会以 `NUL` 结尾；超长文本会被截断。

## ABI 和结构体布局

当前发布端和参考插件都使用 Windows/MSVC 默认结构体对齐。消费者如果使用 C/C++，应直接复制结构体并使用 Windows SDK 类型：`DWORD`、`BOOL`、`ULONGLONG` 和 `wchar_t`。

如果使用 Rust、C#、Go、Python 等语言适配，需要注意：

- `DWORD` 是 32 位无符号整数。
- `BOOL` 是 32 位整数，不是 1 字节布尔值。
- `ULONGLONG` 是 64 位无符号整数。
- Windows `wchar_t` 是 16 位 UTF-16 code unit。
- 字符串数组是固定长度 UTF-16LE 缓冲区，例如 `wchar_t title[256]` 占 `512` 字节。
- 结构体应按 Windows/MSVC 默认对齐读取，不要用紧凑 packed 布局，除非你已经按实际偏移重新计算所有字段。

最稳妥的做法是：消费者侧用同等字段定义读取完整结构体，然后立即复制出需要的字符串和数值。

## 多行歌词语义

修改版 MusicPlayer2 会把同一个时间戳下的多行 LRC 合并为一条主歌词加若干并列副行。例如：

```lrc
[00:04.680]I've been waiting for you
[00:04.680]shi mi tsu i ta ko e ga ma da o mo i da se ru to
[00:04.680]一直等待着你
```

发布结果通常是：

```text
current_lyric          = "I've been waiting for you"
current_parallel_count = 2
current_parallel_1     = "shi mi tsu i ta ko e ga ma da o mo i da se ru to"
current_parallel_2     = "一直等待着你"
current_translate      = 按 MusicPlayer2 本体设置选中的其中一条副行
```

推荐消费者优先使用 `current_parallel_*` 做多行/可选副行显示；如果只支持一条翻译，则使用 `current_translate`。

## 读取示例

```cpp
#include <Windows.h>
#include <optional>

std::optional<Mp2LyricIpcState> ReadMusicPlayer2LyricState()
{
    HANDLE mapping = OpenFileMappingW(FILE_MAP_READ, FALSE, MP2_LYRIC_IPC_NAME);
    if (mapping == nullptr)
        return std::nullopt;

    const auto state_ptr = static_cast<const Mp2LyricIpcState*>(
        MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, sizeof(Mp2LyricIpcState)));
    if (state_ptr == nullptr)
    {
        CloseHandle(mapping);
        return std::nullopt;
    }

    Mp2LyricIpcState state = *state_ptr;
    UnmapViewOfFile(state_ptr);
    CloseHandle(mapping);

    if (state.magic != MP2_LYRIC_IPC_MAGIC || state.version != MP2_LYRIC_IPC_VERSION)
        return std::nullopt;
    if (!state.running)
        return std::nullopt;

    return state;
}
```

如果需要持续读取，可以缓存 `mapping` 和 view 指针，定时复制结构体，并用 `sequence` 判断是否变化。不要直接长期引用共享内存中的字符串指针；应复制到消费者自己的内存后再使用。

## 推荐显示策略

消费者可以按能力选择不同策略。

### 单行显示

```text
current_lyric
```

如果想同时显示翻译，可拼接：

```text
current_lyric + " / " + selected_parallel_line
```

### 双行显示

```text
第 1 行：current_lyric
第 2 行：selected_parallel_line 或 current_translate
```

如果有多条并列副行，推荐提供一个设置：

```text
0 或 -1：显示最后一条副行
1：显示 current_parallel_1
2：显示 current_parallel_2
3：显示 current_parallel_3
```

### 三行显示

```text
第 1 行：current_lyric
第 2 行：current_parallel_1
第 3 行：current_parallel_2
```

如果 `current_parallel_count` 不足，可降级为双行或单行。

### 当前歌词 + 下一句歌词

```text
第 1 行：current_lyric
第 2 行：next_lyric
```

这种模式建议忽略翻译和并列副行，避免任务栏空间过挤。

## 状态新鲜度

MusicPlayer2 会在 UI 绘制歌词时发布状态。消费者可以结合以下字段判断状态是否可用：

- `running == TRUE`
- `magic == MP2_LYRIC_IPC_MAGIC`
- `version == MP2_LYRIC_IPC_VERSION`
- `sequence` 与上次读取不同
- `GetTickCount64() - tick_count` 未超过消费者自定义阈值

如果读取失败、版本不匹配、`running == FALSE` 或状态过旧，消费者应显示空内容、歌曲信息占位，或提示 MusicPlayer2 未运行。

## 兼容性建议

1. 必须校验 `magic` 和 `version`。
2. 不要假设未来版本结构体永远不变。
3. 读取时先复制整个 `Mp2LyricIpcState`，再处理字段。
4. `current_parallel_count` 和 `next_parallel_count` 最大按 `3` 处理，超出时忽略多余值。
5. 如果只支持旧版单翻译显示，使用 `current_translate` 和 `next_translate`。
6. 如果支持多行歌词，优先使用 `current_parallel_*` 和 `next_parallel_*`。
7. 字符串字段可能为空，消费者应做好降级。
8. 字符串字段可能被截断，消费者不应依赖完整歌词全文。

## 当前实现位置

- 发布端：`MusicPlayer2/Mp2LyricIpc.h`
- 发布端：`MusicPlayer2/Mp2LyricIpc.cpp`
- 发布调用：`MusicPlayer2/CUIDrawer.cpp`
- 参考消费端：`TrafficMonitorPlugins/Plugins/MusicPlayer2Lyric/Mp2LyricIpcReader.cpp`

## 许可说明

本接口来自 `MusicPlayer2-TaskbarLyric` 个人修改版。该 fork 基于 `zhongyang219/MusicPlayer2`，应保留上游项目版权和 GPLv3 许可声明。第三方适配者分发修改版 MusicPlayer2 二进制时，应同时提供对应源码。
