#!/usr/bin/env Rscript

# install.packages ('ggplot2')
library(ggplot2)
## # install.packages ('scales')
## library (scales)

args = commandArgs(trailingOnly=TRUE)
data = read.table(args[1], header=T, sep=";")
#data = read.table("/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/scripts/350mResult/mcsUsedHighway350BI100At7.csv", header=T, sep=";")
#data = data[c(1:7,8:nrow(data)),]

data$Algorithm = factor(data$Algorithm)
data$MCS = factor(data$MCS, levels = c("MCS 1", "MCS 2","MCS 3","MCS 4","MCS 5","MCS 6","MCS 7","MCS 8", "MCS 9", "MCS 10", "MCS 11", "MCS 12"))
data$Count = data$Count

g.all<- ggplot(data, aes(fill=Algorithm, x=MCS, y=Count)) +
  geom_bar(width=0.75, position=position_dodge(width=0.8), stat="identity") +
  # geom_text(aes(x = MCS, y = Count, label = Count, group = Algorithm),
  #           position = position_stack(vjust = .5), size = 6, color = "white") +
  scale_x_discrete(breaks=c("MCS 1", "MCS 3","MCS 6","MCS 9", "MCS 12")) +
  scale_y_log10(expand = c(0, 0)) +
  scale_fill_manual(values=c("#04B486","#2E9AFE", "#DBA901", "#5F5C58","#E95E3F", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5")) +

  
  # geom_text(aes(y = lab_ypos, label = len, group =supp), color = "white")
#  geom_line(aes(color=Algorithm), size=1) +
#  geom_point(aes(shape=Algorithm),size=3) +
#  scale_x_discrete(breaks=c("1 a.m.", "5 a.m.","9 a.m.","1 p.m.","5 p.m.","9 p.m.")) +
  #scale_y_discrete(limits=data$Algorithm) +
  #scale_fill_brewer(palette="Blues")+
#  scale_color_manual(name  ="Algorithm", 
#                     values=c("#04B486","#2E9AFE", "#DBA901", "#5F5C58","#E95E3F", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5"))+
#  scale_shape_manual(values=c(15, 16, 17, 18, 19, 8, 9, 11)) +
 # scale_shape_discrete(name  ="Algorithm")+
  #ggtitle("Sweeping time and Data transmission time")+
  #xlab("Beacon interval duration")+
  ylab("Received packets")+
  theme(plot.title = element_text(lineheight=.8, size=20, face="bold", hjust=0.8),
        axis.title.x = element_blank(),
        #axis.title.x = element_text(size=18, vjust=-0.8),
        axis.title.y = element_text(size=18),
        axis.text.x  = element_text(vjust=0.35, size=14),
        axis.text.y  = element_text(vjust=0.35, size=16),
        legend.title= element_blank(),
        legend.background = element_rect(colour = "black"),
        legend.key.size = unit(1, "cm"),
        legend.text = element_text(size = 14))+
  # Uncomment this line for setting the position of legend
  # legend.position=c(.2, 0.4))+
  ggsave(file=args[2], width=8, height=6)
  # ggsave(file="../../../../writting/seemoo-thesis-template/gfx/plots/ieee80211adBeamSweepingTime.pdf", width=8, height=6)
  #ggsave(file="/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/scripts/350mResult/mcsUsedHighway350BI100At7.pdf", width=8, height=6)
  
  print(g.all)
 x = dev.off()
