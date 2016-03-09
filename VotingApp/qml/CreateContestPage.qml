import QtQuick 2.5
import QtQuick.Layouts 1.1
import Qt.labs.controls 1.0
import QtQml 2.2

import VPlayApps 1.0

import QtQmlTricks.UiElements 2.0

import FollowMyVote.StakeWeightedVoting 1.0

Page {
    id: createContestPage
    backgroundColor: "white"

    property VotingSystem votingSystem
    property var contestCreator
    property var purchaseRequest: contestCreator.getPurchaseContestRequest()

    SwipeView {
        id: swiper
        anchors.fill: parent

        BasicContestForm {
            votingSystem: createContestPage.votingSystem
            contestCreator: createContestPage.contestCreator
            purchaseRequest: createContestPage.purchaseRequest
        }
        SponsorshipForm {
            onSponsorshipEnabledChanged: purchaseRequest.sponsorshipEnabled = sponsorshipEnabled
            SwipeView.onIsCurrentItemChanged: {
                if (!SwipeView.isCurrentItem && purchaseRequest.sponsorshipEnabled) {
                    try {
                        purchaseRequest.sponsorMaxVotes = maxVotes
                    } catch (a) {
                        purchaseRequest.sponsorMaxVotes = 0
                    }
                    try {
                        purchaseRequest.sponsorMaxRevotes = maxRevotes
                    } catch (a) {
                        purchaseRequest.sponsorMaxRevotes = 0
                    }
                    try {
                        purchaseRequest.sponsorIncentive = incentive * 10000
                    } catch (a) {
                        purchaseRequest.sponsorIncentive = 0
                    }
                    purchaseRequest.sponsorEndDate = endTime
                }
            }
        }
    }
    PageControl {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        pages: swiper.count
        currentPage: swiper.currentIndex
    }
}
