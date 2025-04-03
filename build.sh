if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is not installed or not in PATH."
    read -p "Press Enter to exit"
    exit 1
fi

if ! command -v git &> /dev/null; then
    echo "Error: Git is not installed or not in PATH."
    read -p "Press Enter to exit"
    exit 1
fi

echo "Running build.cmake..."
cmake -P build.cmake

if [ $? -ne 0 ]; then
    read -p "Press Enter to exit"
fi
exit $?