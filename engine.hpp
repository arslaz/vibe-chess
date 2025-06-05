// engine.hpp
#pragma once
#include <windows.h>
#include <string>
#include <iostream>
#include <chrono>

class ChessEngine {
private:
    HANDLE hChildStd_IN_Rd = NULL;
    HANDLE hChildStd_IN_Wr = NULL;
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;
    PROCESS_INFORMATION piProcInfo;
    bool engineReady = false;

public:
    ChessEngine() = default;

    ~ChessEngine() {
        CloseConnection();
    }

    bool ConnectToEngine(const std::wstring& enginePath = L"stockfish.exe") {
        SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0) ||
            !CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) {
            std::cerr << "CreatePipe failed: " << GetLastError() << std::endl;
            return false;
        }

        STARTUPINFOW siStartInfo = { sizeof(STARTUPINFOW) };
        siStartInfo.hStdError = hChildStd_OUT_Wr;
        siStartInfo.hStdOutput = hChildStd_OUT_Wr;
        siStartInfo.hStdInput = hChildStd_IN_Rd;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        std::wstring cmdLine = L"\"" + enginePath + L"\"";
        if (!CreateProcessW(
            NULL,
            &cmdLine[0],
            NULL,
            NULL,
            TRUE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &siStartInfo,
            &piProcInfo)) {
            std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
            return false;
        }

        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);

        engineReady = true;

        // Initialize Stockfish
        SendCommand("uci");
        SendCommand("isready");

        std::string ready;
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count() < 5000) {
            ready = GetResponse(100);
            if (ready.find("readyok") != std::string::npos) {
                break;
            }
        }

        if (ready.find("readyok") == std::string::npos) {
            std::cerr << "Stockfish initialization failed!" << std::endl;
            return false;
        }

        return true;
    }

    void SendCommand(const std::string& command) {
        if (!engineReady) return;

        std::string cmd = command + "\n";
        DWORD dwWritten;
        WriteFile(hChildStd_IN_Wr, cmd.c_str(), cmd.size(), &dwWritten, NULL);
    }

    std::string GetResponse(int timeoutMs = 5000) {
        const int BUFSIZE = 4096;
        CHAR chBuf[BUFSIZE];
        DWORD dwRead;
        std::string response;

        auto start = std::chrono::steady_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count() < timeoutMs) {
            if (!ReadFile(hChildStd_OUT_Rd, chBuf, BUFSIZE - 1, &dwRead, NULL) || dwRead == 0) {
                continue;
            }

            chBuf[dwRead] = '\0';
            response += chBuf;

            if (response.find("readyok") != std::string::npos ||
                response.find("bestmove") != std::string::npos) {
                break;
            }
        }

        return response;
    }

    std::string getBestMove(const std::string& position, int depth = 15) {
        if (!engineReady) return "";

        SendCommand("position startpos moves " + position);
        SendCommand("go depth " + std::to_string(depth));
        std::string response = GetResponse(10000);

        size_t pos = response.find("bestmove ");
        if (pos != std::string::npos) {
            std::string move = response.substr(pos + 9, 4);
            if (move.length() == 4 &&
                isalpha(move[0]) && isdigit(move[1]) &&
                isalpha(move[2]) && isdigit(move[3])) {
                return move;
            }
        }
        return "";
    }

    void CloseConnection() {
        engineReady = false;

        if (hChildStd_IN_Wr) {
            SendCommand("quit");
            CloseHandle(hChildStd_IN_Wr);
            hChildStd_IN_Wr = NULL;
        }

        if (hChildStd_OUT_Rd) {
            CloseHandle(hChildStd_OUT_Rd);
            hChildStd_OUT_Rd = NULL;
        }

        if (piProcInfo.hProcess) {
            WaitForSingleObject(piProcInfo.hProcess, 1000);
            CloseHandle(piProcInfo.hProcess);
            CloseHandle(piProcInfo.hThread);
            piProcInfo.hProcess = NULL;
            piProcInfo.hThread = NULL;
        }
    }

    void SafeClose() {
        try {
            CloseConnection();
        }
        catch (...) {
            std::cerr << "Error while closing engine" << std::endl;
        }
    }
};