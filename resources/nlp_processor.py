#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import traceback
import msgpack
import re
import cProfile
import pstats
import io
from typing import List, Dict, Any, Union, Optional, Tuple
from collections import Counter

# Suppress specific warnings
import warnings
warnings.filterwarnings("ignore", category=FutureWarning)

# Add debug logging
def debug_log(message: str) -> None:
    with open('debug_output.txt', 'a') as f:
        f.write(f"DEBUG: {message}\n")

def error_log(message: str) -> None:
    with open('debug_output.txt', 'a') as f:
        f.write(f"ERROR: {message}\n")

def status_log(message: str) -> None:
    with open('debug_output.txt', 'a') as f:
        f.write(f"STATUS: {message}\n")

def profile_function(func):
    """Decorator to profile function execution time."""
    def wrapper(*args, **kwargs):
        profiler = cProfile.Profile()
        result = profiler.runcall(func, *args, **kwargs)
        stream = io.StringIO()
        stats = pstats.Stats(profiler, stream=stream)
        stats.sort_stats('cumulative')
        stats.print_stats()
        
        with open('profiling_results.txt', 'w') as f:
            f.write(stream.getvalue())
        
        return result
    return wrapper

try:
    import spacy
    from spacy.lang.en import English
    from spacy.tokens import Doc
    debug_log("Successfully imported required libraries")
except ImportError as e:
    error_log(f"Import error: {str(e)}")
    # If running as standalone, attempt to install dependencies
    import subprocess
    status_log("Installing required dependencies...")
    try:
        subprocess.check_call([sys.executable, "-m", "pip", "install", "spacy", "msgpack"])
        subprocess.check_call([sys.executable, "-m", "spacy", "download", "en_core_web_sm"])
        debug_log("Successfully installed dependencies")
        
        import spacy
        from spacy.lang.en import English
        from spacy.tokens import Doc
    except Exception as e:
        error_log(f"Failed to install dependencies: {str(e)}")
        sys.stdout.buffer.write(msgpack.packb({"error": f"Failed to install dependencies: {str(e)}"}))
        sys.exit(1)

# Load spaCy model
try:
    status_log("Loading spaCy model...")
    nlp = spacy.load("en_core_web_sm")
    debug_log("Successfully loaded spaCy model")
except OSError as e:
    error_log(f"Failed to load spaCy model: {str(e)}")
    # Download the model if not available
    import subprocess
    try:
        status_log("Downloading spaCy model...")
        subprocess.check_call([sys.executable, "-m", "spacy", "download", "en_core_web_sm"])
        nlp = spacy.load("en_core_web_sm")
        debug_log("Successfully downloaded and loaded spaCy model")
    except Exception as e:
        error_log(f"Failed to download spaCy model: {str(e)}")
        sys.stdout.buffer.write(msgpack.packb({"error": f"Failed to download spaCy model: {str(e)}"}))
        sys.exit(1)

@profile_function
def clean_text(text: str) -> str:
    """Clean and format text by removing extra whitespace and normalizing punctuation."""
    # Remove extra whitespace
    text = re.sub(r'\s+', ' ', text.strip())
    # Normalize bullet points and numbering
    text = re.sub(r'^\s*[-•]\s*', '• ', text, flags=re.MULTILINE)
    text = re.sub(r'^\s*(\d+)\.\s*', r'\1. ', text, flags=re.MULTILINE)
    return text

@profile_function
def extract_key_points(doc: Doc, top_n: int = 10) -> List[str]:
    """Extract the most important points from a document."""
    points = set()  # Use a set to avoid duplicates
    
    # Get named entities and noun phrases
    for ent in doc.ents:
        if ent.label_ in ['PERSON', 'ORG', 'GPE', 'PRODUCT', 'EVENT', 'TECH']:
            clean_ent = clean_text(ent.text)
            if len(clean_ent.split()) > 1:  # Only add multi-word entities
                points.add(clean_ent)
    
    # Get important sentences
    sentences = [sent for sent in doc.sents]
    sentence_scores = []
    
    for sent in sentences:
        score = 0
        # Score based on named entities
        score += len([ent for ent in sent.ents if ent.label_ in ['PERSON', 'ORG', 'GPE', 'PRODUCT', 'EVENT', 'TECH']])
        # Score based on important verbs
        score += len([token for token in sent if token.pos_ == 'VERB' and not token.is_stop])
        # Score based on length (prefer medium-length sentences)
        length = len(sent)
        if 10 <= length <= 30:
            score += 2
        # Score based on position (prefer sentences at section starts)
        if sent == sentences[0] or sent.text.strip().endswith(':'):
            score += 3
        # Score based on key terms
        if any(term in sent.text.lower() for term in ['key', 'main', 'important', 'significant']):
            score += 2
        
        clean_sent = clean_text(sent.text)
        sentence_scores.append((clean_sent, score))
    
    # Sort by score and get top sentences
    sorted_sentences = sorted(sentence_scores, key=lambda x: x[1], reverse=True)
    
    # Add top sentences to points, avoiding duplicates and fragments
    for sent, _ in sorted_sentences[:top_n]:
        if sent and len(sent.split()) > 3:  # Only add meaningful sentences
            if not any(sent in p for p in points):  # Avoid adding if it's a subset of existing point
                points.add(sent)
    
    # Convert set to list and sort by length (shorter points first)
    return sorted(list(points), key=len)

@profile_function
def format_bullet_points(points: List[str], indent: str = "") -> str:
    """Format a list of points into bullet points."""
    if not points:
        return ""
    
    formatted = []
    for point in points:
        # Clean the point
        point = clean_text(point)
        # Add bullet point
        formatted.append(f"{indent}• {point}")
    
    return "\n".join(formatted)

@profile_function
def generate_concise_notes(text: str, mode: str = "brief") -> Dict[str, Any]:
    """Generate concise notes from the input text based on the specified mode."""
    try:
        status_log("Processing text...")
    doc = nlp(text)
    
        # Extract key points
        status_log("Extracting key points...")
        key_points = extract_key_points(doc)
        
        # Format based on mode
        if mode == "bullet_points":
            formatted_text = format_bullet_points(key_points)
        else:
            # For brief and detailed modes, join points with newlines
            formatted_text = "\n".join(key_points)
        
        # Create result dictionary with expected field names
        result = {
            "summary": formatted_text,
            "key_concepts": "\n".join([point for point in key_points if len(point.split()) <= 5]),
            "topics": "\n".join([point for point in key_points if len(point.split()) > 5]),
            "success": True
        }
    
        status_log("Processing complete")
        return result
        
    except Exception as e:
        error_log(f"Error in generate_concise_notes: {str(e)}")
        error_log(traceback.format_exc())
        return {
            "error": str(e),
            "success": False
        }

def main():
    """Main entry point for the script."""
    try:
        # Read input from stdin as MessagePack
        input_data = msgpack.unpackb(sys.stdin.buffer.read())
        text = input_data.get(b"text", b"").decode('utf-8')
        mode = input_data.get(b"mode", b"brief").decode('utf-8')
        
        # Process the text
        result = generate_concise_notes(text, mode)
        
        # Write the MessagePack result directly to stdout
        sys.stdout.buffer.write(msgpack.packb(result))
        
    except Exception as e:
        error_msg = f"Error in main: {str(e)}\nTraceback: {traceback.format_exc()}"
        error_log(error_msg)
        sys.stdout.buffer.write(msgpack.packb({
            "error": error_msg,
            "success": False
        }))

if __name__ == "__main__":
    main()

