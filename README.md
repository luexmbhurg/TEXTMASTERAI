# TextMaster

TextMaster is an application that converts  text into concise, well-organized notes. Inspired by tools like Algor Education and HyperWrite's Smart Notes Creator, TextMaster provides powerful summarization capabilities in a desktop application.

## Features

- **Multi-source Input**: Process text, audio recordings, and images
- **Advanced AI Summarization**: Generate concise notes using state-of-the-art NLP
- **Local LLM Support**: Run Gemma, Llama, or other LLMs directly from your device without API keys
- **Customizable Output**: Choose between brief summaries, detailed notes, or bullet points
- **Adjustable Parameters**: Control summary length and readability level
- **User-friendly Interface**: Clean and intuitive Qt-based UI

## Technical Architecture

- **C++ Core**: Performance-optimized main engine
- **Python NLP Backend**: Powerful text processing using spaCy and Hugging Face Transformers
- **llama.cpp Integration**: Run local LLMs like Gemma directly in the application
- **Qt UI**: Professional desktop experience
- **Windows Compatibility**: Packaged as a standalone .exe for Windows

## Building from Source

### Prerequisites

- CMake (3.20 or higher)
- Qt 6 (with Core, Widgets, and Concurrent modules)
- vcpkg for dependency management
- C++ compiler supporting C++17

### Build Instructions

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/textmaster.git
   cd textmaster
   ```

2. Use the provided build script for Windows:
   ```
   build_with_gemma.bat
   ```

   This will:
   - Configure the project with CMake
   - Build the application
   - Create a models directory

3. Download a model:
   - Get a GGUF model file from [Hugging Face](https://huggingface.co/google/gemma-2b-it-gguf/tree/main)
   - Place it in the `build\Release\models` directory
   - We recommend starting with `gemma-2b-it-q4_0.gguf` (approximately 2GB)

4. Run the application:
   ```
   .\build\Release\LessonDecoder.exe
   ```

## Using Local LLMs

TextMaster now supports using local large language models for high-quality study guide generation:

1. **Automatic Model Detection**: The app will look for model files in the `models` directory
2. **Manual Selection**: If no model is found, you'll be prompted to select one
3. **Study Guide Generation**: Enter your text and click "LET'S GO!" to generate a comprehensive study guide
4. **Fallback Mode**: If the local LLM fails, the app will fall back to the Python NLP processor

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [llama.cpp](https://github.com/ggml-org/llama.cpp) for local LLM integration
- [Gemma](https://blog.google/technology/developers/gemma-open-models/) from Google for the open LLM
- [spaCy](https://spacy.io/) for natural language processing
- [Hugging Face Transformers](https://huggingface.co/transformers/) for AI models
- [Qt](https://www.qt.io/) for the user interface framework
