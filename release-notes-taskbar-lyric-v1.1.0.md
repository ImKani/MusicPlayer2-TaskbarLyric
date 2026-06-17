# MusicPlayer2 Taskbar Lyric v1.1.0

这是 MusicPlayer2 的个人修改版源码 release，用于支持 TrafficMonitor 任务栏歌词插件。

普通用户建议从整合发布仓库下载完整包：

```text
https://github.com/ImKani/MusicPlayer2TaskbarLyric
```

## 主要变化

- 增加任务栏歌词共享内存 IPC。
- 向插件发布曲名、歌手、当前歌词、下一句歌词、翻译和同时间戳并列歌词行。
- 支持同时间戳多行 LRC 数据传递。
- 增加“同时间戳并列歌词行”设置，用于选择 MusicPlayer2 本体翻译显示使用哪一行。
- 配合 `MusicPlayer2Lyric` TrafficMonitor 插件实现 Windows 任务栏歌词显示。

## 相关仓库

- TrafficMonitor 歌词插件：`https://github.com/ImKani/TrafficMonitorPlugins-MusicPlayer2Lyric`
- 整合发布仓库：`https://github.com/ImKani/MusicPlayer2TaskbarLyric`

## 版权和说明

Copyright (C) 2026 by ImKani.

本仓库基于 `zhongyang219/MusicPlayer2`。MusicPlayer2 上游项目使用 GPLv3。如果分发修改版 `MusicPlayer2.exe`，应同时提供对应源码。
