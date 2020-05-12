\name{plotarc}
\alias{plotarc}

\title{Plots the data imported from an ARC file}
\description{

Taking as argument the list returned by the get.arcdata function,
this function plots all the arcs. With the argument new the user can decide
whether to plot on a new window/device or on the last window/device.

If the user provides the arguments 'xlim' and 'ylim' they will be used when 
calling the 'plot' function. Other way, the real boundary of the plotted
arcs will be used.
}

\usage{plotarc(arc, new=TRUE, index=NULL, xlim, ylim, ...)}

\arguments{
\item{arc}{The data returned by a call to get.arcdata}
\item{new}{Do you want to plot on the last window/device or on a new one?}
\item{index}{A vector containing the indexes of the arcs to be plotted. If it nos supplied all the arcs will be plotted.}
\item{xlim}{x limits}
\item{ylim}{y limits}
\item{...}{Options to be passed to a call to the function plot when
creating the display (i. e., window or file).}
}

\value{
This function returns nothing, but plots a nice map :-D.
}

\keyword{hplot}
