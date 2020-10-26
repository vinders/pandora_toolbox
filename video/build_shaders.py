#!/usr/bin/env python3

# --- SHADER BUILDER ---
# author: Romain Vinders
# description: build shaders from shader modules: assemble them (#include"..." / #include<...>) + remove white-spaces and comments

import os
import sys
import glob
import shutil
import subprocess

# Get first child directory
def getFirstChildDir(dirPath):
  for path in glob.glob(dirPath + '/*/'):
    if path!='.' and path!='..':
      return path
  return ''
#


# Find Vulkan SDK path
def findVulkanSdk(generationOutputPath):
  vulkanSdkPath = os.getenv('VULKAN_SDK', default='')
  if vulkanSdkPath!='' and os.path.exists(vulkanSdkPath):
    if os.path.exists(os.path.join(vulkanSdkPath, 'x86_64/bin')):
      vulkanSdkPath = os.path.join(vulkanSdkPath, 'x86_64')
  else:
    if os.name=='nt':
      if os.path.exists('C:\\VulkanSDK'):
        vulkanSdkPath = getFirstChildDir('C:\\VulkanSDK')
    else:
      projectRootPathIndex = generationOutputPath.rfind('/video')
      projectRoot = generationOutputPath[0:projectRootPathIndex] if projectRootPathIndex>-1 else os.path.join(generationOutputPath, '../..')
    
      if os.path.exists('/usr/local/x86_64/bin/glslangValidator'):
        vulkanSdkPath = '/usr/local/x86_64'
      elif os.path.exists('/usr/local/bin/glslangValidator'):
        vulkanSdkPath = '/usr/local'
      elif os.path.exists('/usr/local/VulkanSDK/'):
        vulkanSdkPath = getFirstChildDir('/usr/local/VulkanSDK/') + '/x86_64'
      elif os.path.exists('/usr/VulkanSDK/'):
        vulkanSdkPath = getFirstChildDir('/usr/VulkanSDK/') + '/x86_64'
      elif os.path.exists(os.path.join(projectRoot, '_libs/VulkanSDK')):
        vulkanSdkPath = getFirstChildDir(os.path.join(projectRoot, '_libs/VulkanSDK')) + '/x86_64'
      else:
        homeDir = os.getenv('HOME', default='/home')
        if os.path.exists(homeDir + '/VulkanSDK'):
          vulkanSdkPath = getFirstChildDir(homeDir + '/VulkanSDK') + '/x86_64'
  return vulkanSdkPath
#


# Verify if a file is missing or outdated
def isFileOutdated(fileName, fileDir, outShaderPath, outModulePath):
  if os.path.exists(os.path.join(outShaderPath, fileName)):
    if os.path.getmtime(os.path.join(fileDir, fileName)) > os.path.getmtime(os.path.join(outShaderPath, fileName)):
      return True
  if os.path.exists(os.path.join(outModulePath, fileName)):
    if os.path.getmtime(os.path.join(fileDir, fileName)) > os.path.getmtime(os.path.join(outModulePath, fileName)):
      return True
  else:
    return True
  return False
#


# Remove white-spaces and comments
def trimLine(fileLine):
  line = fileLine.strip(' \t\r\n')
  
  # single-line comments
  commentIndex = line.find('//')
  if commentIndex>-1:
    if commentIndex==0:
      return '',False
    elif line.rfind('"')<commentIndex:
      return line[0:commentIndex],False
  
  # multi-line comments
  commentIndex = line.find('/*')
  if commentIndex==0 or (commentIndex>-1 and line.find('"')==-1):
    lineLeft = '' if commentIndex==0 else line[0:commentIndex]
    commentEndIndex = line.find('*/', commentIndex+2)
    if commentEndIndex>-1:
      if commentEndIndex>=len(line)-1:
        return lineLeft,False
      else:
        return lineLeft + line[commentEndIndex+2:],False
    else:
      return lineLeft,True

  return line,False
#


# Get file path for included sub-file
def readIncludeFilePath(line, srcFileName):
  # get file name/path limits
  isLocalFile = True
  startIndex = line.find('"');
  endIndex = -1
  if startIndex>-1:
    endIndex = line.find('"', startIndex+1);
  else:
    startIndex = line.find("<");
    endIndex = line.find(">", startIndex+1);
    isLocalFile = False
  
  if endIndex<startIndex:
    print(srcFileName + ': warning: ' + line + ': missing end symbol...')
    endIndex = len(line)
  
  # extract file path
  relativePath = line[startIndex+1:endIndex]
  if endIndex-startIndex>2 and relativePath[0]=='.' and relativePath[1]=='/':
    relativePath = relativePath[2:]
  return relativePath,isLocalFile
#


# Verify and format data for file inclusion
def processFileIncludeData(includeRelativePathInfo, outFile, srcFileDir, outShaderPath, outModulePath, outStringPath, installDirs):
  fileNameIndex = includeRelativePathInfo[0].rfind('/') + 1
  fileName = includeRelativePathInfo[0][fileNameIndex:]
  relativeDirPath = includeRelativePathInfo[0][0:fileNameIndex] if fileNameIndex>0 else ''
  
  fileDirPath = os.path.join(srcFileDir, relativeDirPath)
  outSubShaderPath = os.path.join(outShaderPath, relativeDirPath) if relativeDirPath!='' else outShaderPath
  outSubModulePath = os.path.join(outModulePath, relativeDirPath) if relativeDirPath!='' else outModulePath
  outSubStringPath = os.path.join(outStringPath, relativeDirPath) if relativeDirPath!='' else outStringPath
  
  # non-local file include
  if includeRelativePathInfo[1]==False and len(installDirs)>0:
    for dir in installDirs:
      if os.path.exists(os.path.join(dir, includeRelativePathInfo[0])):
        fileDirPath = os.path.join(dir, relativeDirPath)
        break

  # verify path validity
  if not os.path.exists(os.path.join(fileDirPath, fileName)):
    print(includeRelativePathInfo[0] + ': not found in ' + srcFileDir, file=sys.stderr)
    exit(1)
    
  return fileName,fileDirPath,outSubShaderPath,outSubModulePath,outSubStringPath
#


# Stringify assembled shader file
def stringifyShaderFile(shaderFilePath, outStringFilePath):
  with open(shaderFilePath, 'r') as inFile:
    inFile.seek(0) 
    with open(outStringFilePath, 'w') as outFile:
      outFile.write('R"(')
      outFile.write(inFile.read()) 
      outFile.write(')"\n')
#
  

# Generate assembled shader file
def generateShaderFile(srcFileName, srcFileDir, outShaderPath, outModulePath, outStringPath, installDirs):
  isShaderFile = False
  isInCommentBlock = False
  outFilePath = os.path.join(outModulePath, srcFileName)
  with open(outFilePath, 'w') as outFile:
  
    # read source file line by line
    with open(os.path.join(srcFileDir, srcFileName), 'r') as inFile:
      inFile.seek(0) 
      for fileLine in inFile:
        if isInCommentBlock:
          commentEndIndex = fileLine.find('*/')
          if commentEndIndex>=0: # end of comment block
            isInCommentBlock = False
            fileLine = fileLine[commentEndIndex+2:]
          else:
            continue
        
        lineInfo = trimLine(fileLine)
        isInCommentBlock = lineInfo[1]
        if lineInfo[0]:
          # sub-file including
          if lineInfo[0][0:8]=='#include':
            includeRelativePathInfo = readIncludeFilePath(lineInfo[0], srcFileName) #returns: relativePath, isLocalPath
            fileIncludeData = processFileIncludeData(includeRelativePathInfo, outFile, srcFileDir, outShaderPath, outModulePath, outStringPath, installDirs)
            #returns: name, dirPath, shaderSubPath, moduleSubPath, stringSubPath
            
            # generate sub-file data
            isShaderSubFile = False
            outSubFilePath = os.path.join(fileIncludeData[3], fileIncludeData[0])
            if isFileOutdated(fileIncludeData[0], fileIncludeData[1], fileIncludeData[2], fileIncludeData[3]):
              print(' > include: ' + includeRelativePathInfo[0] + ': generating...')
              outSubFilePath = generateShaderFile(fileIncludeData[0], fileIncludeData[1], fileIncludeData[2], fileIncludeData[3], fileIncludeData[4], installDirs)
            else:
              print(' > include: ' + includeRelativePathInfo[0] + ': up to date')
              if not os.path.exists(outSubFilePath):
                outSubFilePath = os.path.join(fileIncludeData[2], fileIncludeData[0])
                isShaderSubFile = True
            
            # copy sub-file data
            with open(outSubFilePath, 'r') as inSubFile:
              outFile.write(inSubFile.read())
            outFile.write('\n')
            if isShaderSubFile:
              isShaderFile = True
          
          # not an include -> write trimmed line in output file
          else:
            if isShaderFile==False and lineInfo[0].find('main(')>=0: # file contains entry point
              isShaderFile = True
            outFile.write(lineInfo[0] + '\n')

  if isShaderFile==True:
    if not os.path.exists(outShaderPath):
      os.mkdir(outShaderPath)
    elif os.path.exists(os.path.join(outShaderPath, srcFileName)):
      os.unlink(os.path.join(outShaderPath, srcFileName))
    shutil.move(outFilePath, outShaderPath)
    outFilePath = os.path.join(outShaderPath, srcFileName)
  print('   generated in: ' + outFilePath)
  
  outSerializerPath = os.path.join(outStringPath, srcFileName + '.inc')
  stringifyShaderFile(outFilePath, outSerializerPath)
  print('   serialized in: ' + outSerializerPath)
  return outFilePath
#


# ---


# read directory arguments
print('--- Shader builder ---')
if len(sys.argv) < 3:
  print('Missing directory argument(s) (usage: build_shader.py "source_dir" "shader_output_dir" "string_output_dir")', file=sys.stderr)
  exit(1)
if sys.version_info[0] < 3 or sys.version_info[1] < 4:
  print("This script requires Python 3.4 or greater.")
  exit(1)

srcPath = os.path.abspath(sys.argv[1])
outShaderPath = os.path.abspath(sys.argv[2])
outModulePath = os.path.join(outShaderPath, 'modules')
outStringPath = os.path.abspath(sys.argv[3])
installDirsList = []
if len(sys.argv)>4:
  for dir in sys.argv[4:]:
    installDirsList.append(os.path.abspath(dir))

if not os.path.exists(srcPath):
  print('-- Empty or non-existing source directory.')
  exit(0)
  
# create output directories
if not os.path.exists(outShaderPath):
  os.makedirs(outShaderPath, exist_ok=True)
if not os.path.exists(outModulePath):
  os.makedirs(outModulePath, exist_ok=True)
if not os.path.exists(outStringPath):
  os.makedirs(outStringPath, exist_ok=True)

# generate data
for root,dirs,files in os.walk(srcPath, topdown=True):
  print('-- Directory: ' + root)
  relSrcPath = os.path.relpath(root, srcPath) 
  if os.name=='nt':
    relSrcPath = relSrcPath.replace('/','\\')
  absShaderPath = os.path.join(outShaderPath, relSrcPath) if relSrcPath!='.' else outShaderPath
  absModulePath = os.path.join(outModulePath, relSrcPath) if relSrcPath!='.' else outModulePath
  absStringPath = os.path.join(outStringPath, relSrcPath) if relSrcPath!='.' else outStringPath

  for dir in dirs:
    if not os.path.exists(os.path.join(absModulePath, dir)):
      os.mkdir(os.path.join(absModulePath, dir))
    if not os.path.exists(os.path.join(absStringPath, dir)):
      os.mkdir(os.path.join(absStringPath, dir))

  for file in files:
    # shader missing or outdated -> generate + stringify
    if isFileOutdated(file, root, absShaderPath, absModulePath):
      print(' - ' + file + ': generating...')
      generateShaderFile(file, root, absShaderPath, absModulePath, absStringPath, installDirsList)
    # shader up to date, but no stringified version -> stringify
    elif not os.path.exists(os.path.join(absStringPath, file + '.inc')): 
      print(' - ' + file + ': up to date')
      if os.path.exists(os.path.join(absShaderPath, file)):
        stringifyShaderFile(os.path.join(absShaderPath, file), os.path.join(absStringPath, file + '.inc'))
      if os.path.exists(os.path.join(absModulePath, file)):
        stringifyShaderFile(os.path.join(absModulePath, file), os.path.join(absStringPath, file + '.inc'))


# compile vulkan shaders to SPIR-V (if Vulkan SDK available)
vulkanShaderPath = os.path.join(outShaderPath, 'vulkan')
if os.path.exists(vulkanShaderPath):
  vulkanSdkPath = findVulkanSdk(outShaderPath)
  vulkanSdkCompiler = os.path.join(vulkanSdkPath,'Bin\\glslangValidator.exe') if os.name=='nt' else os.path.join(vulkanSdkPath,'bin/glslangValidator')
  if os.path.exists(vulkanSdkCompiler):
    print('-- Compiling Vulkan shaders to SPIR-V...')

    # detect files to compile
    for root,dirs,files in os.walk(vulkanShaderPath, topdown=True):
      relPath = os.path.relpath(root, vulkanShaderPath) 
      if os.name=='nt':
        relPath = relPath.replace('/','\\')
      absShaderPath = os.path.join(vulkanShaderPath, relPath) if relPath!='.' else vulkanShaderPath
      for file in files:
        if file[-4:]=='.spv':
          continue
        filePath = os.path.join(absShaderPath, file)
        if os.path.exists(filePath + '.spv'):
          os.unlink(filePath + '.spv')
        
        # compile command
        print(' - ' + vulkanSdkCompiler + ' -V "' + filePath + '" -o "' + filePath + '.spv"')
        process = subprocess.Popen([vulkanSdkCompiler, '-V', filePath, '-o', filePath + '.spv'])
        process.wait()
        if not os.path.exists(filePath + '.spv'):
          print(' > failed')

exit(0)
