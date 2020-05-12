\name{get.bnddata}
\alias{get.bnddata}

\title{Function for importing the contents of a BND file into R}
\description{
This function reads and imports into R the  contents of a BND file. This kind
of files store the bounds of the given coverage (the one we are working
with). That is, the upper and lower bounds for the x and y coordinates.
}

\usage{get.bnddata(infodir, tablename)}

\arguments{
\item{infodir}{Directory where there is a file called arc.dat (usually, it is called 'info').}
\item{tablename}{The name of the table in the coverage that
stores the data (usually called 'COVERAGENAME.BND').}
}


\value{
A vector with the  x min, y min, x max and y max values.
}


\keyword{file}
