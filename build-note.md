**1. Install Sphinx 2.4.5 (Need python3):**  
```console
pip install -r Doc/requirements.txt
```
**2. Install VS2022's v141, v141_xp toolset and v141 ATL**  
**3. Install Strawberry Perl**  
**4. Then run:**
```console
set PYTHON=xxx (The Python executable with Sphinx 2.4.5 installed)
buildrelease.bat -o out -x86 --skip-nuget
```
**Optional: If you want to build Windows Appx package:**  
```Run the following on python source root using cmd
set APPX_DATA_PUBLISHER=<your certificate subject name>
set APPX_DATA_SHA256=<your certificate SHA256>
```
- When building 32bit-Python, you should run this with 64bit-Python:
```cmd
python3 Tools/msi/getpackagefamilyname.py
set APPX_PACKAGEFAMILYNAME=<output of last cmd>
python PC/layout --copy win32 --include-appxmanifest
```
- Then run the following using powershell:
```powershell
Tools/msi/make_appx.ps1 win32 python.msix
```
- Finally run this using cmd:
```cmd
signtool sign /fd sha256 /a /f <certificate file> /p <certificate password> /tr http://time.certum.pl /td SHA256 python.msix
```
