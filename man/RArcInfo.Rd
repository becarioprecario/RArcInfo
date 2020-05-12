\name{RArcInfo}
\alias{RArcInfo}


\title{RArcInfo}
\description{
This package allows the user to import into R binary coverages in format
Arc/Info V 7.x. These coverages represent geographical data in several forms:
points, lines, polygons, point labels, etc. 

 RArcInfo uses the library AVCE00, written by Daniel Morissette, to whom I would
thank fo his marvelous work. But RArcInfo is much more than a wrapper of this library, because
it provides functions to plot the data and draw maps.

 Since the geographical data are separated into several files, RArcInfo provides a different 
function to read each file. These functions are called get.XXXdata, where XXX is the
file name to open (usually the extension is 'adf'):

\describe{
\item{get.arcdata}{ARC files contain arcs definition and their vertices.}

\item{get.bnddata}{BND files contain coordinates for the boundary of the data.}

\item{get.cntdata}{CNT files contain polygon centroid information.}

\item{get.labdata}{LAB files contain label point records.}

\item{get.paldata}{PAL files contain the polygon definitions.}

\item{get.toldata}{TOL files contain the tolerance values that were used when processing the polygon coverage.}

\item{get.txtdata}{TXT files contain annotations (or labels) about the data.}
}


 Besides these files, binary coverage store several tables containing additional information
(like name of the city, population, etc.). To get this data, RArcInfo provides the
next functions:
\describe{


\item{get.tablenames}{Gets all the table names and the coverage each table belongs to.}

\item{get.tablefields}{Gets the names of the fields for a given table.}

\item{get.tabledata}{Gets the data stored in the table.}


}

In order to plot the data, RArcInfo has several functions:

\describe{

\item{plotarc}{Plots all the arcs.}

\item{plotpal}{Plots all the polygons.}

\item{plotpoly}{Like plotpal, but it allows to select the polygons we want to plot, colour and other stuff. This is useful to plot maps according the value of some covariate.}
}


To get all the names of the coverages, the user can call 'get.namesofcoverages'.
Other two interesting functions are:

\describe{
\item{thinlines}{Useful to reduce the number of points in an arc according to a given tolerance.}

\item{get.nb}{Calculates the neighbouring polygons of a given set of polygons.}
}
}


\seealso{get.arcdata, get.bnddata, get.cntdata, get.labdata, get.paldata,
get.toldata, get.txtdata, get.tablenames, get.tablefields, get.tabledata,
get.namesofcoverages, read.coverage, thinlines, get.nb }


\references{
V. Gómez-Rubio and A. López-Quílez (2005). RArcInfo: Using GIS data
  with R. Computers & Geosciences 3(8), 1000-1006.

More information about this kind of data can be found at
\url{http://avce00.maptools.org/docs/v7_bin_cover.html}.

}

\examples{

#See example(plotpoly) for another example

library(RArcInfo)

datadir<-system.file("exampleData",package="RArcInfo")
infodir<-system.file("exampleData","info",package="RArcInfo")
coveragedir<-system.file("exampleData","wetlands",package="RArcInfo")

#get.bnddata needs the last slash...
infodir<-paste(c(infodir,"/"), collapse="")

#List all the tables

covnames<-get.namesofcoverages(datadir)
tablenames<-get.tablenames(infodir)


#Display the name of the table and its fields
for(i in 1:length(tablenames[[1]]))
{
	print(c("Table: ",tablenames$TableName[i]))

	fields<-get.tablefields(infodir,tablenames$TableName[i])
	print("Fields")
	for(j in 1:length(fields))
		print(fields[[j]][1])

	#Get the data
	if(i==1)
		tabledata<-get.tabledata(infodir,tablenames$TableName[i])
	else
		tabledata<-c(tabledata, get.tabledata(infodir,tablenames$TableName[i]) )
}

#Import data from some tables
arc<-get.arcdata(datadir,"wetlands")
pal<-get.paldata(datadir,"wetlands")
lab<-get.labdata(datadir,"wetlands")
cnt<-get.cntdata(datadir,"wetlands")

bnd<-get.bnddata(infodir,"WETLANDS.BND")

print("Plotting all the arcs")
plotarc(arc)

print("Plotting the first ten polygons (in red) on the previous plot")
par(col="red")
plotpal(arc,pal,new=FALSE, index=1:10)
}

\keyword{file}
