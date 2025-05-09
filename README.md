# TextMaster AI

TextMaster AI is a powerful desktop application that uses AI to generate study materials from text input. It leverages the TinyLlama model to create comprehensive study guides, quizzes, flashcards, and enumerations.

## Features

- Generate study guides from any text input
- Create quizzes with multiple choice questions
- Generate flashcards for quick review
- Create enumerations of key points
- Modern Qt-based user interface
- Local AI processing using llama.cpp
- History tracking of generated content

## Requirements

- Windows 10 or later
- CMake 3.15 or later
- Qt 6.5 or later
- Visual Studio 2019 or later with C++17 support
- At least 4GB of RAM
- 1GB of free disk space

## Setup

1. Clone the repository:
```bash
git clone https://github.com/yourusername/textmaster-ai.git
cd textmaster-ai
```

2. Build the project:
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

3. Download the TinyLlama model:
- Download the Q4_K_M quantized version of TinyLlama from Hugging Face
- Place it in the `build/Release/models` directory

4. Run the application:
```bash
cd Release
./TextMaster.exe
```

## Usage

1. Launch TextMaster AI
2. Enter or paste your text in the input area
3. Choose the type of study material you want to generate:
   - Study Guide
   - Quiz
   - Flashcards
   - Enumerations
4. Click "Generate" and wait for the AI to process your text
5. View and save your generated study materials

## License

MIT License

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
