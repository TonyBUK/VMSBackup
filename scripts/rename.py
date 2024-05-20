import os

kFileList = os.listdir()

kCurrentPath = os.path.abspath(os.curdir)

for kFile in kFileList :

    if os.path.isfile(kFile) :

        #! TODO: What if the save set has multiple "drives"
        if "[" in kFile and "]" in kFile :

            kPath       = kFile[:kFile.find("]") + 1]
            kPath       = kPath[(kFile.find("[") + 1):-1].split(".")
            kFileNoPath = kFile[kFile.find("]") + 1:]

            kTargetPath  = os.path.join(kCurrentPath, os.path.join(*[j for j in kPath]))
            kTargetFile  = os.path.join(kTargetPath, kFileNoPath)

            if not os.path.isdir(kTargetPath) :
                os.makedirs(kTargetPath)
            #end
                
            os.rename(kFile, kTargetFile)

        #end

    #end

#end