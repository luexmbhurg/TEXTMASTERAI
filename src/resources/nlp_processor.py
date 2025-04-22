#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import traceback
import json
import re
import tempfile
import warnings
from typing import List, Dict, Any, Union, Optional
from collections import Counter

# Suppress specific warnings
warnings.filterwarnings("ignore", category=FutureWarning, module="transformers")

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

# Initialize models
summarizer = None
sentiment_analyzer = None

def get_summarizer():
    model_name = "facebook/bart-large-cnn"
    tokenizer = AutoTokenizer.from_pretrained(model_name)
    model = AutoModelForSeq2SeqLM.from_pretrained(model_name)
    return pipeline("summarization", model=model, tokenizer=tokenizer)

def get_sentiment_analyzer():
    return pipeline("sentiment-analysis", model="distilbert-base-uncased-finetuned-sst-2-english")

def calculate_optimal_length(text: str) -> int:
    """Calculate optimal summary length based on input text length."""
    word_count = len(text.split())
    # For very short texts, use a higher ratio
    if word_count < 50:
        return min(50, word_count)
    # For medium texts, use about 1/3 of the length
    elif word_count < 200:
        return max(50, word_count // 3)
    # For longer texts, use about 1/4 of the length
    else:
        return max(100, word_count // 4)

def clean_text(text: str) -> str:
    """Clean and format text by removing extra whitespace and normalizing punctuation."""
    # Remove extra whitespace
    text = re.sub(r'\s+', ' ', text.strip())
    # Normalize bullet points and numbering
    text = re.sub(r'^\s*[-•]\s*', '• ', text, flags=re.MULTILINE)
    text = re.sub(r'^\s*(\d+)\.\s*', r'\1. ', text, flags=re.MULTILINE)
    return text

def extract_topics(doc: Doc, num_topics: int = 5) -> List[Dict[str, Any]]:
    """Extract main topics from the document using noun phrases and named entities."""
    # Collect noun phrases and their frequencies
    noun_phrases = []
    for chunk in doc.noun_chunks:
        if not all(token.is_stop for token in chunk):
            noun_phrases.append(chunk.text.lower())
    
    # Collect named entities
    entities = [ent.text.lower() for ent in doc.ents if ent.label_ in ['ORG', 'PRODUCT', 'GPE', 'EVENT', 'TECH']]
    
    # Combine and count frequencies
    all_topics = noun_phrases + entities
    topic_counter = Counter(all_topics)
    
    # Get the most common topics
    top_topics = []
    for topic, count in topic_counter.most_common(num_topics):
        # Get the sentiment of the sentences containing this topic
        topic_sentences = [sent.text for sent in doc.sents if topic in sent.text.lower()]
        topic_sentiment = "neutral"
        if topic_sentences:
            sentiment = analyze_sentiment(" ".join(topic_sentences[:3]))  # Analyze first 3 sentences
            topic_sentiment = sentiment["label"]
        
        top_topics.append({
            "topic": topic,
            "frequency": count,
            "sentiment": topic_sentiment
        })
    
    return top_topics

def analyze_sentiment(text: str) -> Dict[str, Any]:
    """Analyze the sentiment of the text."""
    global sentiment_analyzer
    if sentiment_analyzer is None:
        sentiment_analyzer = get_sentiment_analyzer()
    
    result = sentiment_analyzer(text)[0]
    return {
        "label": result["label"].lower(),
        "score": round(result["score"], 3)
    }

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

def extract_key_concepts(doc: Doc) -> List[Dict[str, Any]]:
    """Extract key concepts and their definitions from the document."""
    concepts = []
    current_concept = None
    current_definition = []
    
    # Common definition patterns
    definition_patterns = [
        r"([A-Za-z0-9\s]+)\s+(?:is|are)\s+(?:defined as|refers to|means|known as)\s+(.+)",
        r"([A-Za-z0-9\s]+)\s+(?:is|are)\s+(?:a|an)\s+(.+)",
        r"([A-Za-z0-9\s]+)\s+(?:is|are)\s+(.+)",
        r"([A-Za-z0-9\s]+)\s*:\s*(.+)"
    ]
    
    # First pass: Look for explicit definitions
    for sent in doc.sents:
        text = sent.text.strip()
        for pattern in definition_patterns:
            matches = re.finditer(pattern, text, re.IGNORECASE)
            for match in matches:
                concept = match.group(1).strip()
                definition = match.group(2).strip()
                # Clean up the concept and definition
                concept = re.sub(r'^\d+\.\s*', '', concept)  # Remove numbering
                definition = re.sub(r'\.$', '', definition)  # Remove trailing period
                
                if len(concept.split()) <= 5 and len(definition.split()) >= 3:  # Reasonable length checks
                    concepts.append({
                        "concept": concept,
                        "definition": definition,
                        "context": text
                    })
    
    # Second pass: Look for implicit definitions in numbered lists
    for sent in doc.sents:
        text = sent.text.strip()
        # Check for numbered list items
        if re.match(r'^\d+\.\s+', text):
            # Try to split into concept and definition
            parts = re.split(r'\.\s+', text, 1)
            if len(parts) == 2:
                concept = parts[0].replace(r'^\d+\.\s*', '').strip()
                definition = parts[1].strip()
                if len(concept.split()) <= 5 and len(definition.split()) >= 3:
                    concepts.append({
                        "concept": concept,
                        "definition": definition,
                        "context": text
                    })
    
    # Remove duplicates while preserving order
    seen = set()
    unique_concepts = []
    for concept in concepts:
        key = (concept["concept"].lower(), concept["definition"].lower())
        if key not in seen:
            seen.add(key)
            unique_concepts.append(concept)
    
    return unique_concepts

def extract_formulas(doc: Doc) -> List[Dict[str, Any]]:
    """Simple formula extraction focusing only on basic equations."""
    formulas = []
    for sent in doc.sents:
        # Look for basic equations with = sign
        matches = re.finditer(r'([A-Za-z0-9]+)\s*=\s*([A-Za-z0-9+\-*/^()]+)', sent.text)
        for match in matches:
            formula = match.group()
            formulas.append({
                "formula": formula,
                "context": sent.text
            })
    return formulas

def extract_practice_questions(doc: Doc) -> List[Dict[str, Any]]:
    """Extract potential practice questions from the document."""
    questions = []
    question_indicators = ["what", "how", "why", "explain", "describe", "calculate"]
    
    for sent in doc.sents:
        # Check if sentence starts with a question indicator
        if any(sent.text.lower().startswith(indicator) for indicator in question_indicators):
            questions.append({
                "question": sent.text,
                "type": "conceptual" if any(ind in sent.text.lower() for ind in ["what", "how", "why"]) else "practical"
            })
    
    return questions

def format_bullet_points(points: List[str], indent: str = "") -> str:
    """Format a list of points with proper indentation and bullet points."""
    formatted_points = []
    current_section = None
    
    for point in points:
        # Clean the point text
        clean_point = clean_text(point)
        
        # Handle section headers
        if clean_point.endswith(':'):
            current_section = clean_point
            formatted_points.append(f"{indent}• {clean_point}")
            continue
        
        # Handle numbered lists
        if re.match(r'^\d+\.', clean_point):
            number = re.match(r'^(\d+)\.', clean_point).group(1)
            content = re.sub(r'^\d+\.\s*', '', clean_point)
            if current_section:
                formatted_points.append(f"{indent}  {number}. {content}")
    else:
                formatted_points.append(f"{indent}• {number}. {content}")
            continue
        
        # Handle nested points
        if ':' in clean_point and '\n' not in clean_point:
            main_point, sub_points = clean_point.split(':', 1)
            # Format main point
            if current_section:
                formatted_points.append(f"{indent}  • {main_point.strip()}:")
            else:
                formatted_points.append(f"{indent}• {main_point.strip()}:")
            # Format sub-points with additional indentation
            if sub_points.strip():
                sub_items = [s.strip() for s in sub_points.split(',') if s.strip()]
                for sub_item in sub_items:
                    formatted_points.append(f"{indent}    - {sub_item}")
        else:
            # Regular bullet point
            if current_section:
                formatted_points.append(f"{indent}  • {clean_point}")
            else:
                formatted_points.append(f"{indent}• {clean_point}")
    
    return "\n".join(formatted_points)

def generate_concise_notes(text: str, mode: str = "brief") -> Dict[str, Any]:
    """Generate concise notes from input text with additional analysis."""
    global summarizer
    if summarizer is None:
        summarizer = get_summarizer()
    
    # Process text with spaCy
    doc = nlp(text)
    
    # Calculate optimal summary length
    max_length = calculate_optimal_length(text)
    min_length = max(30, max_length // 2)
    
    # Extract study-specific elements
    key_concepts = extract_key_concepts(doc)
    formulas = extract_formulas(doc)  # Simplified formula extraction
    practice_questions = extract_practice_questions(doc)
    topics = extract_topics(doc)
    
    # Generate content based on mode
    if mode == "brief":
        # Generate a brief summary
        summary = summarizer(
            text, 
            max_length=max_length, 
            min_length=min_length, 
            do_sample=False,
            num_beams=4,
            length_penalty=2.0
        )[0]['summary_text']
        
        return {
            "summary": clean_text(summary),
            "key_concepts": key_concepts[:5],  # Top 5 concepts
            "formulas": formulas,
            "topics": topics,
            "word_count": len(text.split()),
            "success": True
        }
        
    elif mode == "detailed":
        # Generate a detailed summary with key points
        summary = summarizer(
            text, 
            max_length=max_length, 
            min_length=min_length, 
            do_sample=False,
            num_beams=4,
            length_penalty=2.0
        )[0]['summary_text']
        
        key_points = extract_key_points(doc)
        formatted_points = format_bullet_points(key_points)
        
        return {
            "summary": clean_text(summary),
            "key_points": formatted_points,
            "key_concepts": key_concepts,
            "formulas": formulas,
            "practice_questions": practice_questions,
            "topics": topics,
            "word_count": len(text.split()),
            "success": True
        }
        
    elif mode == "bullet_points":
        # Generate bullet points only
        key_points = extract_key_points(doc)
        formatted_points = format_bullet_points(key_points)
        
        return {
            "key_points": formatted_points,
            "key_concepts": key_concepts,
            "formulas": formulas,
            "practice_questions": practice_questions,
            "topics": topics,
            "word_count": len(text.split()),
            "success": True
        }
    
    else:
        return {"error": "Invalid mode selected"}

def process_text(text: str, mode: str) -> str:
    """Main processing function for text input."""
    try:
        # Validate input length
        if len(text.split()) > 1000:
            return json.dumps({
                "error": "Input text exceeds 1000 words limit",
                "word_count": len(text.split()),
                "success": False
            })
        
        # Validate mode
        valid_modes = ["brief", "detailed", "bullet_points"]
        if mode not in valid_modes:
            return json.dumps({
                "error": f"Invalid mode. Must be one of: {', '.join(valid_modes)}",
                "success": False
            })
        
        # Generate notes based on mode
        result = generate_concise_notes(text, mode)
        result["word_count"] = len(text.split())
        result["success"] = True
        
        # Ensure all required fields are present
        required_fields = ["summary", "key_concepts", "formulas", "topics", "word_count", "success"]
        for field in required_fields:
            if field not in result:
                result[field] = None if field != "success" else False
        
        return json.dumps(result, indent=2)
        
    except Exception as e:
        return json.dumps({
            "error": str(e),
            "traceback": traceback.format_exc(),
            "success": False
        })

def main():
    if len(sys.argv) != 3:
        print(json.dumps({
            "error": "Invalid number of arguments. Expected: mode text",
            "success": False
        }))
        return
    
    mode = sys.argv[1]
    text = sys.argv[2]
    
    result = process_text(text, mode)
    print(result)

if __name__ == "__main__":
    main()
