#pragma once

#include <filesystem>
#include <system_error>

namespace redactly
{
    inline bool isWithinRoot(const std::filesystem::path &candidate,
                             const std::filesystem::path &root)
    {
        std::error_code ec;
        const auto relative = std::filesystem::relative(candidate, root, ec);
        if (ec || relative.empty())
        {
            return false;
        }
        const auto first = relative.begin();
        return first != relative.end() && *first != "..";
    }

    inline bool destinationIsSafe(const std::filesystem::path &destination,
                                  const std::filesystem::path &safeRoot)
    {
        std::error_code ec;
        auto current = destination;
        while (!current.empty() && current != current.root_path())
        {
            if (std::filesystem::exists(current, ec))
            {
                if (std::filesystem::is_symlink(current, ec))
                {
                    auto resolved = std::filesystem::canonical(current, ec);
                    if (ec || !isWithinRoot(resolved, safeRoot))
                    {
                        return false;
                    }
                }
                break;
            }
            current = current.parent_path();
        }

        if (std::filesystem::exists(destination, ec))
        {
            auto resolved = std::filesystem::canonical(destination, ec);
            if (ec || !isWithinRoot(resolved, safeRoot))
            {
                return false;
            }
        }

        const auto lexical = destination.lexically_normal();
        return isWithinRoot(lexical, safeRoot) || lexical == safeRoot.lexically_normal();
    }
}
