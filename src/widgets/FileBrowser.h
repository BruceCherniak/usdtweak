#include <string>
#include <vector>
#include <set>

/// Draw a file browser ui which can be used anywhere, except that there is only one instance of the FileBrowser
/// for the application. There is no OK/Cancel button, or options, this is the responsibility of the caller to draw them.
void DrawFileBrowser(int height);

/// Returns the current stored file browser path
std::string GetFileBrowserFilePath();

/// Returns the current stored file browser path, relative to root
std::string GetFileBrowserFilePathRelativeTo(const std::string& root, bool unixify=false);

/// Make sure the file path has an extension, if not "ext" is append as an extension
void EnsureFileBrowserDefaultExtension(const std::string& ext);

/// Make sure the file extension is ext
void EnsureFileBrowserExtension(const std::string& ext);

/// Reset currently stored path in the file browser
void ResetFileBrowserFilePath();

/// Set the filebrowser directory to look at
void SetFileBrowserDirectory(const std::string &directory);

/// Set the filebrowser directory to look at
void SetFileBrowserFilePath(const std::string &path);

/// Set the filebrowser directory to look at
std::string GetFileBrowserDirectory();

/// Returns true if the current file browser path exists
bool FilePathExists();

/// Sets the valid extensions. The file browser will filter out any files with an invalid extension.
void SetValidExtensions(const std::vector<std::string> &extensions);
