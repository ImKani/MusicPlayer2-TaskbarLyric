#include "stdafx.h"
#include "Mp2LyricIpc.h"

CMp2LyricIpc& CMp2LyricIpc::Instance()
{
    static CMp2LyricIpc instance;
    return instance;
}

CMp2LyricIpc::~CMp2LyricIpc()
{
    Clear();
}

bool CMp2LyricIpc::EnsureOpen()
{
    if (m_state != nullptr)
        return true;

    m_mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
        sizeof(Mp2LyricIpcState), MP2_LYRIC_IPC_NAME);
    if (m_mapping == nullptr)
        return false;

    m_state = static_cast<Mp2LyricIpcState*>(
        MapViewOfFile(m_mapping, FILE_MAP_WRITE, 0, 0, sizeof(Mp2LyricIpcState)));
    if (m_state == nullptr)
    {
        CloseHandle(m_mapping);
        m_mapping = nullptr;
        return false;
    }

    ZeroMemory(m_state, sizeof(Mp2LyricIpcState));
    m_state->magic = MP2_LYRIC_IPC_MAGIC;
    m_state->version = MP2_LYRIC_IPC_VERSION;
    m_state->process_id = GetCurrentProcessId();
    m_state->running = TRUE;
    return true;
}

void CMp2LyricIpc::Publish(const std::wstring& title, const std::wstring& artist, const std::wstring& current_lyric,
    const std::wstring& current_translate, const std::vector<std::wstring>& current_parallel_lines,
    const std::wstring& next_lyric, const std::wstring& next_translate, const std::vector<std::wstring>& next_parallel_lines,
    int progress, bool playing, bool has_lyric)
{
    if (!EnsureOpen())
        return;

    m_state->magic = MP2_LYRIC_IPC_MAGIC;
    m_state->version = MP2_LYRIC_IPC_VERSION;
    m_state->sequence = ++m_sequence;
    m_state->process_id = GetCurrentProcessId();
    m_state->tick_count = GetTickCount64();
    m_state->running = TRUE;
    m_state->playing = playing;
    m_state->has_lyric = has_lyric;
    m_state->progress = progress;
    CopyText(m_state->title, _countof(m_state->title), title);
    CopyText(m_state->artist, _countof(m_state->artist), artist);
    CopyText(m_state->current_lyric, _countof(m_state->current_lyric), current_lyric);
    CopyText(m_state->current_translate, _countof(m_state->current_translate), current_translate);
    CopyText(m_state->next_lyric, _countof(m_state->next_lyric), next_lyric);
    CopyText(m_state->next_translate, _countof(m_state->next_translate), next_translate);
    m_state->current_parallel_count = static_cast<DWORD>(current_parallel_lines.size() < 3 ? current_parallel_lines.size() : 3);
    m_state->next_parallel_count = static_cast<DWORD>(next_parallel_lines.size() < 3 ? next_parallel_lines.size() : 3);
    CopyText(m_state->current_parallel_1, _countof(m_state->current_parallel_1), current_parallel_lines.size() > 0 ? current_parallel_lines[0] : L"");
    CopyText(m_state->current_parallel_2, _countof(m_state->current_parallel_2), current_parallel_lines.size() > 1 ? current_parallel_lines[1] : L"");
    CopyText(m_state->current_parallel_3, _countof(m_state->current_parallel_3), current_parallel_lines.size() > 2 ? current_parallel_lines[2] : L"");
    CopyText(m_state->next_parallel_1, _countof(m_state->next_parallel_1), next_parallel_lines.size() > 0 ? next_parallel_lines[0] : L"");
    CopyText(m_state->next_parallel_2, _countof(m_state->next_parallel_2), next_parallel_lines.size() > 1 ? next_parallel_lines[1] : L"");
    CopyText(m_state->next_parallel_3, _countof(m_state->next_parallel_3), next_parallel_lines.size() > 2 ? next_parallel_lines[2] : L"");
}

void CMp2LyricIpc::Clear()
{
    if (m_state != nullptr)
    {
        m_state->running = FALSE;
        m_state->playing = FALSE;
        m_state->has_lyric = FALSE;
        m_state->sequence = ++m_sequence;
        UnmapViewOfFile(m_state);
        m_state = nullptr;
    }
    if (m_mapping != nullptr)
    {
        CloseHandle(m_mapping);
        m_mapping = nullptr;
    }
}

void CMp2LyricIpc::CopyText(wchar_t* dest, size_t count, const std::wstring& src)
{
    if (count == 0)
        return;
    wcsncpy_s(dest, count, src.c_str(), _TRUNCATE);
}
