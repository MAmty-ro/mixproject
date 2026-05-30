import os
import subprocess
import re
import sys

def pulisci_nome_file(titolo):
    return re.sub(r'[\\/*?:"<>|]', "", titolo)

def avvia_pipeline_download(url_video):
    cartella_cache = "Progetto_Mixer/cache_audio"
    os.makedirs(cartella_cache, exist_ok=True)
    
    # 1. Configurazione comandi di yt-dlp per estrarre informazioni e traccia audio
    comando_info = ["yt-dlp", "--get-title", url_video]
    
    try:
        titolo_grezzo = subprocess.check_output(comando_info, text=True).strip()
        titolo_pulito = pulisci_nome_file(titolo_grezzo)
    except Exception as e:
        print(f"Errore estrazione info video: {e}")
        return None

    percorso_output_template = os.path.join(cartella_cache, f"{titolo_pulito}.%(ext)s")
    percorso_wav_finale = os.path.join(cartella_cache, f"{titolo_pulito}.wav")
    
    comando_download = [
        "yt-dlp",
        "-x",
        "--audio-format", "wav",
        "--audio-quality", "0",
        "-o", percorso_output_template,
        url_video
    ]
    
    try:
        subprocess.run(comando_download, check=True)
        # Standardizzazione forzata a CD Audio 44100Hz, Stereo, 16-bit PCM
        percorso_tmp = percorso_wav_finale + ".tmp.wav"
        os.rename(percorso_wav_finale, percorso_tmp)
        
        comando_ffmpeg = [
            "ffmpeg", "-y", "-i", percorso_tmp,
            "-ar", "44100", "-ac", "2", "-c:a", "pcm_s16le",
            percorso_wav_finale
        ]
        subprocess.run(comando_ffmpeg, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
        os.remove(percorso_tmp)
        
        return percorso_wav_finale
    except Exception as e:
        print(f"Errore durante il download o la conversione: {e}")
        return None

if __name__ == '__main__':
    # Se la GUI ci passa l'URL come argomento a riga di comando
    if len(sys.argv) > 1:
        url_video = sys.argv[1]
        if "youtube.com" in url_video or "youtu.be" in url_video:
            percorso_output = avvia_pipeline_download(url_video)
            if percorso_output:
                print(f"RISULTATO_SUCCESS: {percorso_output}")
            else:
                print("RISULTATO_ERROR")
        sys.exit(0)
        
    # Modalità manuale classica da terminale (se lanciato da solo)
    print("--- TEST MODULO DOWNLOADER ---")
    while True:
        url_video = input("Inserisci l'URL del video di YouTube (o 'esci'): ")
        if url_video.lower() == 'esci':
            break
        if "youtube.com" in url_video or "youtu.be" in url_video:
            print("Link valido! Avvio la pipeline di download...")
            avvia_pipeline_download(url_video)
        else:
            print("URL non valido. Riprova.")