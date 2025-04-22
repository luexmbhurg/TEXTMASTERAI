
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import traceback
import json
import re
import tempfile
from typing import List, Dict, Any, Union, Optional

try:
    import spacy
    from spacy.lang.en import English
    from spacy.tokens import Doc
    import numpy as np
    from transformers import pipeline, AutoTokenizer, AutoModelForSeq2SeqLM
except ImportError:
    # If running as standalone, attempt to install dependencies
    import subprocess
    print("Installing required dependencies...", file=sys.stderr)
    subprocess.check_call([sys.executable, "-m", "pip", "install", "spacy", "numpy", "transformers", "torch"])
    subprocess.check_call([sys.executable, "-m", "spacy", "download", "en_core_web_sm"])
    
    import spacy
    from spacy.lang.en import English
    from spacy.tokens import Doc
    import numpy as np
    from transformers import pipeline, AutoTokenizer, AutoModelForSeq2SeqLM

# Load spaCy model
try:
    nlp = spacy.load("en_core_web_sm")
except OSError:
    # Download the model if not available
    import subprocess
    subprocess.check_call([sys.executable, "-m", "spacy", "download", "en_core_web_sm"])
    nlp = spacy.load("en_core_web_sm")

# Initialize summarization model
def get_summarizer():
    model_name = "facebook/bart-large-cnn"
    tokenizer = AutoTokenizer.from_pretrained(model_name)
    model = AutoModelForSeq2SeqLM.from_pretrained(model_name)
    summarizer = pipeline("summarization", model=model, tokenizer=tokenizer)
    return summarizer

# Initialize globals
summarizer = None

def extract_keywords(doc: Doc, top_n: int = 10) -> List[str]:
    """Extract the most important keywords from a document."""
    # Extract noun phrases and named entities
    keywords = []
    
    # Get named entities
    for ent in doc.ents:
        keywords.append(ent.text)
    
    # Get important noun chunks
    for chunk in doc.noun_chunks:
        if not any(chunk.text.lower() in keyword.lower() for keyword in keywords):
            keywords.append(chunk.text)
    
    # Get important single tokens (nouns, verbs, adjectives)
    important_pos = {'NOUN', 'PROPN', 'VERB', 'ADJ'}
    for token in doc:
        if token.pos_ in important_pos and not any(token.text.lower() in keyword.lower() for keyword in keywords):
            if not token.is_stop and len(token.text) > 1:
                keywords.append(token.text)
    
    # Remove duplicates and sort by frequency
    keyword_counts = {}
    for keyword in keywords:
        keyword_lower = keyword.lower()
        if keyword_lower in keyword_counts:
            keyword_counts[keyword_lower] += 1
        else:
            keyword_counts[keyword_lower] = 1
    
    # Sort by count, then by length (prefer longer keywords)
    sorted_keywords = sorted(
        list(set(keywords)), 
        key=lambda x: (
            -keyword_counts[x.lower()],  # Higher count first
            -len(x)                       # Longer words first
        )
    )
    
    return sorted_keywords[:top_n]

def segment_text(text: str, max_length: int = 1024) -> List[str]:
    """Split text into segments that don't exceed the model's max token limit."""
    # Use spaCy to split by sentences
    doc = nlp(text)
    sentences = [sent.text for sent in doc.sents]
    
    segments = []
    current_segment = []
    current_length = 0
    
    for sentence in sentences:
        # Approximate token count (rough estimate)
        sentence_length = len(sentence.split())
        
        if current_length + sentence_length > max_length:
            # Current segment is full, start a new one
            segments.append(" ".join(current_segment))
            current_segment = [sentence]
            current_length = sentence_length
        else:
            # Add to current segment
            current_segment.append(sentence)
            current_length += sentence_length
    
    # Add the last segment if it's not empty
    if current_segment:
        segments.append(" ".join(current_segment))
    
    return segments

def generate_summary(text: str, mode: str, max_length: int = 150) -> str:
    """Generate a summary based on the specified mode."""
    global summarizer
    
    if not summarizer:
        summarizer = get_summarizer()
    
    # Parse the mode string
    mode_parts = mode.split('_')
    summary_type = mode_parts[0]  # 'brief', 'detailed', or 'bullets'
    
    summary_length = 3  # default
    if len(mode_parts) > 1:
        try:
            summary_length = int(mode_parts[1])
        except ValueError:
            pass
    
    readability_level = 3  # default (1=elementary, 5=advanced)
    if len(mode_parts) > 2:
        try:
            readability_level = int(mode_parts[2])
        except ValueError:
            pass
    
    # Adjust max length based on summary type
    if summary_type == 'brief':
        max_length = min(100, max_length)
    elif summary_type == 'detailed':
        max_length = max(200, max_length)
    
    # Adjust max length based on summary length setting (paragraphs)
    max_length = max_length * summary_length // 3
    
    # Segment the text to fit into the model
    segments = segment_text(text)
    segment_summaries = []
    
    for segment in segments:
        if len(segment.strip()) == 0:
            continue
            
        try:
            summary = summarizer(
                segment, 
                max_length=max_length, 
                min_length=max(30, max_length // 4),
                do_sample=False
            )
            segment_summaries.append(summary[0]['summary_text'])
        except Exception as e:
            print(f"Error summarizing segment: {str(e)}", file=sys.stderr)
            traceback.print_exc(file=sys.stderr)
            # If summarization fails, include the first part of the segment
            segment_summaries.append(segment[:min(len(segment), 100)] + "...")
    
    # Combine segment summaries
    combined_summary = " ".join(segment_summaries)
    
    # Post-process based on summary type
    if summary_type == 'bullets':
        # Convert to bullet points
        doc = nlp(combined_summary)
        bullet_points = []
        
        for sent in doc.sents:
            # Clean and format the sentence
            sentence = sent.text.strip()
            if sentence:
                bullet_points.append(f"• {sentence}")
        
        return "\n".join(bullet_points)
    else:
        # Apply readability adjustments
        if readability_level <= 2:
            # Simplify language for elementary level
            combined_summary = simplify_text(combined_summary)
        elif readability_level >= 4:
            # Use more sophisticated language
            combined_summary = enhance_text(combined_summary)
        
        return combined_summary

def simplify_text(text: str) -> str:
    """Simplify text for lower reading levels."""
    doc = nlp(text)
    simplified_sentences = []
    
    for sent in doc.sents:
        # Shorter sentences are generally easier to read
        if len(sent) > 20:  # If sentence is long
            # Try to break it at a conjunction or comma
            sub_sentences = re.split(r'(?<=[.,;])\s+|(?<=\w)\s+(?:and|but|or|because|since)\s+(?=\w)', sent.text)
            simplified_sentences.extend(sub_sentences)
        else:
            simplified_sentences.append(sent.text)
    
    # Rejoin with appropriate spacing
    return " ".join(simplified_sentences)

def enhance_text(text: str) -> str:
    """Enhance text for higher reading levels (academic)."""
    # This is a simplified implementation
    # In a real application, you might use synonyms, restructuring, etc.
    
    # Add a formal introduction
    introduction = "The following summarizes the key points from the provided content. "
    
    # Add a formal conclusion
    conclusion = " The preceding summary encapsulates the essential information contained in the original text."
    
    return introduction + text + conclusion

def extract_main_points(text: str, num_points: int = 5) -> List[str]:
    """Extract the main points from a text."""
    doc = nlp(text)
    
    # Score sentences based on keywords, position, and length
    keywords = extract_keywords(doc)
    keyword_set = {k.lower() for k in keywords}
    
    scored_sentences = []
    for i, sent in enumerate(doc.sents):
        # Skip very short sentences
        if len(sent) < 5:
            continue
            
        # Calculate score based on keywords
        keyword_score = sum(1 for token in sent if token.text.lower() in keyword_set)
        
        # Position score (earlier sentences often more important)
        position_score = 1.0 / (i + 1)
        
        # Length score (prefer medium length sentences)
        length = len(sent)
        length_score = 1.0 if 10 <= length <= 30 else 0.7
        
        total_score = (keyword_score * 0.5) + (position_score * 0.3) + (length_score * 0.2)
        scored_sentences.append((sent.text, total_score))
    
    # Sort by score and return top points
    scored_sentences.sort(key=lambda x: x[1], reverse=True)
    return [sent for sent, score in scored_sentences[:num_points]]

def process_text(text: str, mode: str) -> str:
    """Process text based on the specified mode."""
    try:
        # Basic input validation
        if not text or len(text.strip()) == 0:
            return "Error: No text provided for processing."
        
        mode = mode.lower()
        if not mode:
            mode = "brief_3_3"  # Default mode
        
        # Generate summary
        result = generate_summary(text, mode)
        
        # Return the result
        return result
        
    except Exception as e:
        error_msg = f"Error processing text: {str(e)}"
        print(error_msg, file=sys.stderr)
        traceback.print_exc(file=sys.stderr)
        return f"An error occurred during processing: {str(e)}"

def main():
    """Main function to handle command-line usage."""
    if len(sys.argv) < 3:
        print("Usage: python nlp_processor.py <mode> <text>", file=sys.stderr)
        print("Modes: brief_X_Y, detailed_X_Y, bullets_X_Y", file=sys.stderr)
        print("  X = summary length (1-10)", file=sys.stderr)
        print("  Y = readability level (1-5)", file=sys.stderr)
        return 1
    
    mode = sys.argv[1]
    text = sys.argv[2]
    
    result = process_text(text, mode)
    print(result)
    return 0

if __name__ == "__main__":
    sys.exit(main())
