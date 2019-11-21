# Testing guidelines for new releases of RNAStructViz

## Checklist of GUI operations to check are still functional after code changes

### Testing the LHS MainWindow pane of the GUI



### Testing the RHS FolderWindow pane of the GUI (after selecting a sequence folder)


## Other testing methodologies to try

Try one's best to "*break it*" by running new combinations and sequences of GUI operations, one after the other, 
to see if we can generate errors. The heuristic here is to say that of the definite checklist of functional GUI 
specs passes testing, and we cannot break the updated code, then it should be safe to impart our best judgement to 
users that the new release will work well for them. At least in princple, this is the best we can hope for without 
painstaking mocking of objects, which would probably be a futile attempt to do with FLTK at any rate.

