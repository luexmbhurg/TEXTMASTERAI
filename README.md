
# TextMaster

TextMaster is an application that converts  text into concise, well-organized notes. Inspired by tools like Algor Education and HyperWrite's Smart Notes Creator, TextMaster provides powerful summarization capabilities in a desktop application.

## Features

- **Advanced Summarization**: Generate concise notes using state-of-the-art NLP
- **Customizable Output**: Choose between brief summaries, detailed notes, or bullet points
- **Adjustable Parameters**: Control summary length and readability level
- **User-friendly Interface**: Clean and intuitive Qt-based UI

## Technical Architecture

- **C++ Core**: Performance-optimized main engine
- **Python NLP Backend**: Powerful text processing using spaCy and Hugging Face Transformers
- **Qt UI**: Professional desktop experience
- **Windows Compatibility**: Packaged as a standalone .exe for Windows

## Building from Source

### Prerequisites

- CMake (3.10 or higher)
- Qt 5 (with Core, Widgets, Multimedia, and MultimediaWidgets modules)
- Python 3 (with development headers)
- C++ compiler supporting C++17

### Build Instructions

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/textmaster.git
   cd textmaster
   ```

2. Create a build directory:
   ```
   mkdir build
   cd build
   ```

3. Configure with CMake:
   ```
   cmake ..
   ```

4. Build the project:
   ```
   cmake --build .
   ```

5. For Windows .exe packaging:
   ```
   cmake --build . --target package
   ```

## Usage

1. Launch the application
2. Choose your input method (text, audio, or image)
3. Import or record your content
4. Adjust summary options as desired
5. Click "Generate Notes" to create your summary
6. Save or copy the results

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [spaCy](https://spacy.io/) for natural language processing
- [Hugging Face Transformers](https://huggingface.co/transformers/) for AI models
- [Qt](https://www.qt.io/) for the user interface framework
