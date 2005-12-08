class SetupController < ApplicationController

  def upgrade
    # no doubt, there will be special cases to these steps for 
    # special files and circumstances, but this is the general procedure
    # for all files.
    #   	
    # pseudo code
    # 1. expand selected snapshot into temp dir
    # 2. append .restore to all files
    # 3. copy *.restore files into system, replacing any existing *.restore files
    #     this will create set of partnerings with original files (e.g. file.dat 
    #     file.dat.restore)
    # 4. delete .restore files that are identical to partner file
    # 4b. delete .restore files w/no partner. Assume file was obsoleted. Have
    #    list of exceptions-to-this-rule files were *.restore ext. drops
    # 5. perform diff between partner files and then patch between the 2
    # 6. if patch fails, add file to list of conflicts
    # 7. if patch succeeds, remove .restore file
    # 8. if any conflicts, move on to resolve them
    # 9. if not. move on to success page.
  end

  def resolve_conflict
    # show partnering files in text windows side-by-side, restore file read-only
    # if save, take edited file
    # if ignore restore file, delete *.restore file and go back to upgrade
    # if ignore existing file, mv *.restore  into orig file and go back to upgrade
  end
  
end
