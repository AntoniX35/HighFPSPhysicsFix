//#include <string>

std::string to_native( const std::wstring& w_str )
{
    if (!w_str.length())
        return "";

    int w_str_length = static_cast<int>(w_str.length());
    int ansi_length = WideCharToMultiByte(CP_UTF8, 0, w_str.c_str(), w_str_length, nullptr, 0, nullptr, nullptr);

    std::unique_ptr<char[]> u_ptr(new char[ansi_length]);
    WideCharToMultiByte(CP_UTF8, 0, w_str.c_str(), w_str_length, u_ptr.get(), ansi_length, nullptr, nullptr);

    return std::string(u_ptr.get(), ansi_length);
}
