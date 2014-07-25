copy /Y ..\Workspace\tracker\xianranlow-result.xml ..\Workspace\tracker\xianranlow-original.xml
python process_result.py ..\Workspace\tracker\xianranlow-original.xml ..\Workspace\tracker\xianranlow-result.xml
copy /Y ..\Workspace\tracker\clip-result.xml ..\Workspace\tracker\clip-original.xml
python process_result.py ..\Workspace\tracker\clip-original.xml ..\Workspace\tracker\clip-result.xml
copy /Y ..\Workspace\tracker\two_short_low-result.xml ..\Workspace\tracker\two_short_low-original.xml
python process_result.py ..\Workspace\tracker\two_short_low-original.xml ..\Workspace\tracker\two_short_low-result.xml
pause