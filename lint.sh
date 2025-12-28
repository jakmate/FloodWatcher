#!/bin/bash

set -e

# Use local Qt installation
export PATH="$QT_DIR/bin:$PATH"

echo "Running qmllint..."
qmllint-qt6 qml/main.qml qml/MapArea.qml qml/StationPanel.qml qml/WarningsPanel.qml

echo "Running qmlformat..."
qmlformat-qt6 -i qml/main.qml qml/MapArea.qml qml/StationPanel.qml qml/WarningsPanel.qml

echo "Running clang-format..."
clang-format -i src/*.cpp include/*.hpp tests/cpp/unit/*.cpp tests/cpp/mocks/*.hpp

echo "Running clang-tidy..."
run-clang-tidy -p build -j $(nproc) \
  -header-filter='^(?!.*(_autogen|\.moc)).*(include/.*\.hpp$)' \
  '.*(src/.*\.cpp$|tests/cpp/mocks/.*\.hpp$|tests/cpp/unit/.*\.cpp$)'

echo "All checks complete!"