import pytesseract
from PIL import Image
import io
import os

# Point pytesseract to the standard Windows installation path
if os.name == 'nt':
    pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'


def extract_text_from_image(image_bytes: bytes) -> str:
    """
    Extracts text from an image using Tesseract OCR.
    Ensure tesseract is installed on the system.
    """
    try:
        image = Image.open(io.BytesIO(image_bytes))
        
        # Preprocessing to optimize OCR:
        # 1. Convert to grayscale
        image = image.convert('L')
        
        # 2. Upscale 2x using Bicubic interpolation (helps Tesseract read smaller/blurry text)
        width, height = image.size
        image = image.resize((width * 2, height * 2), Image.BICUBIC)
        
        # 3. Increase contrast significantly
        from PIL import ImageEnhance
        contrast_enhancer = ImageEnhance.Contrast(image)
        image = contrast_enhancer.enhance(2.0)
        
        # 4. Apply a slight sharpening filter
        sharpness_enhancer = ImageEnhance.Sharpness(image)
        image = sharpness_enhancer.enhance(1.5)
        
        text = pytesseract.image_to_string(image)
        return text.strip()
    except Exception as e:
        print(f"OCR Error: {e}")
        return ""
