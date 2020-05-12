\name{plotpoly}
\alias{plotpoly}

\title{Plots polygons defined by the coverages.}

\description{
This function is capable of plotting polygons, referenced by its id number.
These can be filled with colors acording to a given variate.
}

\usage{plotpoly(arc,bnd,pal,index=NULL,col, border=NULL,xratio=1, yratio=1,...)}

\arguments{
\item{arc}{The data returned by a call to get.arcdata}
\item{bnd}{The data returned by a call to get.bnddata}
\item{pal}{The data returned by a call to get.paldata}
\item{index}{IDs of the polygon to be plotted. If it is 'NULL' then all the polygons are plotted.}
\item{col}{Colors to be used when filling the polygons}
\item{border}{Colors used for the lines of the polygons. If it's not set, it is set to the value of 'col'.}
\item{xratio}{Controls x-axis ratio. It can take any value from 0 to 1.}
\item{yratio}{Controls x-axis ratio. It can take any value from 0 to 1.}
\item{...}{Options to be passed to a call to the function plot when
creating the display (i. e., window or file).}
}

\value{
Thins function returns nothing, but plots a nice map. :-D
}

\examples{
#This example is the same as the one provided under the test directory

#1.- Create temporary directory (if needed)
#1.- Extract the E00 file form a ZIP file
#2.- Create an Arc/Info binary coverage
#3.- Create the map

#get current working directory
cwd<-getwd()
#Create tmp directory.
tmpdir<-tempdir()


datadir<-system.file("exampleData",package="RArcInfo")
setwd(datadir)
file.copy(c("valencia.zip", "data_valencia.csv"), tmpdir, overwrite = TRUE)

setwd(tmpdir)

#Convert E00 file to a binary covertage to be imported into R

#Comment this line if the file valencia.e00 already exists
unzip(zipfile="valencia.zip", file="valencia.e00")

#Comments this lines if the binary coverage already exists
library(RArcInfo)
e00toavc("valencia.e00", "valencia")


#Read map data
arcsmuni<-get.arcdata(".", "valencia")
palmuni<-get.paldata(".", "valencia")
bnd.muni<-get.bnddata("info/", "VALENCIA.BND")
patmuni<-get.tabledata("./info", "VALENCIA.PAT")

#Number of polygons
nmuni<-length(palmuni[[1]][[1]])

municipios<-data.frame(1:nmuni, patmuni$"VALENCIA-ID")
names(municipios)<-c("INDEX", "CODMUNICI")


#Datafiles to be used

unemp<-read.table(file="data_valencia.csv", sep=";", 
	dec = ",",skip=1)

unemp<-unemp[,c(1,3)]
names(unemp)<-c("CODMUNICI", "UNEMP")

breaks<-quantile(unemp[,2], c(0,  .025,.2, .8, .975, 1))

unemp<-cbind(unemp, CAT=as.ordered(cut(unemp[,2], breaks, include.lowest = TRUE) ))


library(RColorBrewer)

#Colors to be used in maps
#colors<-brewer.pal(5, "Oranges")
colors<-brewer.pal(5, "Greens")

ldata<-merge(unemp, municipios, by.x="CODMUNICI", by.y="CODMUNICI")


#Valencia
idx<-(ldata$"CODMUNICI">=46000)
bnd.muni<-c(626679.9, 4250000, 760000, 4460000)


p<-par()
pin<-1.5*p$pin
main<-"Rate of unemployment"

plotpoly(arc=arcsmuni, pal=palmuni, bnd=bnd.muni,
	index=ldata$INDEX[idx], col=colors[ldata$CAT][idx],
	xlab="", ylab="", main=main,
	xaxt="n", yaxt="n", bty="n")

#Set legend
l<-levels(unemp$CAT)
l[1]<-"[0.00,1.26]"
legend(700000, 4460000, fill=colors, 
legend=l, bty="n", cex=1)

setwd(cwd)
}


\keyword{hplot}
