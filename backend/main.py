from fastapi import FastAPI, UploadFile, File, HTTPException
import uvicorn
from .ocr_service import extract_text_from_image

app = FastAPI(title="Braille OCR Backend")

@app.post("/scan")
async def scan_image(file: UploadFile = File(...)):
    if not file.content_type.startswith('image/'):
        raise HTTPException(status_code=400, detail="File provided is not an image.")

    try:
        contents = await file.read()
        text = extract_text_from_image(contents)
        
        # Perform basic text cleaning or validation if necessary
        clean_text = " ".join(text.split())
        
        return {"text": clean_text}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
