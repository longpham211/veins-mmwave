#!/usr/bin/env Rscript

# install.packages ('ggplot2')
library(ggplot2)
library(ggpubr)
## # install.packages ('scales')
## library (scales)

args = commandArgs(trailingOnly=TRUE)
#data = read.table(args[1], header=T, sep=";")
# dataI = read.table("/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/scripts/350mResult/channelUsageIEEE80211adHighway350BI100At7.csv", header=T, sep=";")
# dataS = read.table("/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/scripts/350mResult/channelUsageSAMBAHighway350BI100At7.csv", header=T, sep=";")
# dataF = read.table("/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/scripts/350mResult/channelUsageFMLHighway350BI100At7.csv", header=T, sep=";")

dataF = read.table(args[1], header=T, sep=";")
dataI = read.table(args[2], header=T, sep=";")
dataS = read.table(args[3], header=T, sep=";")

dataF$Type = factor(dataF$Type, levels = c ("Transmission", "Unused and overhead"))
dataI$Type = factor(dataI$Type, levels = c ("Transmission", "Unused and overhead"))
dataS$Type = factor(dataS$Type, levels = c ("Transmission", "Unused and overhead"))

pdataF<- ggplot(dataF, aes(fill=Type, x="", y=Proportion)) +
  geom_bar(width=1, stat="identity") +
  coord_polar("y", start=0) + geom_text(aes(label = paste(Proportion, "%")), color="white", size=4, position = position_stack(vjust = 0.5)) +
  scale_fill_manual(values=c("#04B486","#696969", "#DBA901", "#5F5C58","#E95E3F", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5")) +
  ylab("FML") +
  theme(legend.position="none",
        axis.title.y = element_blank(),
        axis.text = element_blank(),
        axis.ticks = element_blank(),
        panel.grid = element_blank(),
        legend.title = element_blank())

pdataI<- ggplot(dataI, aes(fill=Type, x="", y=Proportion)) +
  geom_bar(width=1, stat="identity") +
  coord_polar("y", start=0) + geom_text(aes(label = paste(Proportion, "%")), color="white", size=4, position = position_stack(vjust = 0.5)) +
  scale_fill_manual(values=c("#04B486","#696969", "#DBA901", "#5F5C58","#E95E3F", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5")) +
  ylab("ieee802.11ad") +
  theme(legend.position="none",
        axis.title.y = element_blank(),
        axis.text = element_blank(),
        axis.ticks = element_blank(),
        panel.grid = element_blank(),
        legend.title = element_blank())

pdataS<- ggplot(dataS, aes(fill=Type, x="", y=Proportion)) +
  geom_bar(width=1, stat="identity") +
  coord_polar("y", start=0) + geom_text(aes(label = paste(Proportion, "%")), color="white", size=4, position = position_stack(vjust = 0.5)) +
  scale_fill_manual(values=c("#04B486","#696969", "#DBA901", "#5F5C58","#E95E3F", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5")) +
  ylab("SAMBA") +
  theme(legend.position="none",
        axis.title.y = element_blank(),
        axis.text = element_blank(),
        axis.ticks = element_blank(),
        panel.grid = element_blank(),
        legend.title = element_blank())


  p <- ggarrange(pdataF, pdataI, pdataS, nrow=3, ncol=1, common.legend = TRUE, legend="bottom", heights = c(0.5,0.5)) 

  #ggsave(file="/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/scripts/350mResult/channelUsageHighway350BI100At7.pdf", p, width=8, height=4)
  ggsave(file=args[4], p, width=3.5, height=8)
  #print(g.all)
  x = dev.off()
