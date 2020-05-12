\name{get.tabledata}
\alias{get.tabledata}

\title{Function for importing  the contents of a table file into R}
\description{
This function reads and imports into R the  contents of a table file.
}

\usage{get.tabledata(infodir, tablename)}

\arguments{
\item{infodir}{Info directory where there is a file called arc.dat}
\item{tablename}{The name of the table from which we want to import the data}
}

\value{
This function returns a data frame in which each column stores the
data of a field from the table. The columns are not assigned the names
of the fields, but this can be got with the get.tablefields function.
}


\keyword{file}
