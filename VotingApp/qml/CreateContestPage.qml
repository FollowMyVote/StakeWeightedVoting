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

    Connections {
        target: contestCreator
        property var lastErrorTime
        onError: {
            // Ignore errors that come within 1 second of the last error
            console.log(lastErrorTime)
            if (new Date().getTime() - lastErrorTime < 1000)
                return

            var message = errorString.split(';')
            if (message.length > 1)
                message = message[1]
            else
                message = errorString

            NativeDialog.confirm("Error purchasing contest", message, function(){}, false)
            lastErrorTime = new Date().getTime()
        }
    }

    SwipeView {
        id: swiper
        anchors.fill: parent

        BasicContestForm {
            id: basicForm
            contestLimits: contestCreator.contestLimits
            coinsModel: votingSystem.coins
            onCompleted: swiper.currentIndex++
        }
        SponsorshipForm {
            onSponsorshipEnabledChanged: purchaseRequest.sponsorshipEnabled = sponsorshipEnabled
            onCompleted: {
                try {
                    var purchaseRequest = contestCreator.getPurchaseContestRequest()
                    purchaseRequest.contestType = ContestType.OneOfN
                    purchaseRequest.tallyAlgorithm = TallyAlgorithm.Plurality
                    purchaseRequest.name = basicForm.contestName
                    purchaseRequest.description = basicForm.contestDescription
                    for (var i = 0; i < basicForm.contestantModel.count; i++)
                        purchaseRequest.contestants.append(basicForm.contestantModel.get(i))
                    purchaseRequest.expiration = basicForm.contestExpiration
                    purchaseRequest.weightCoin = basicForm.weightCoinId
                    if (purchaseRequest.sponsorshipEnabled) {
                        purchaseRequest.sponsorMaxVotes = maxVotes? maxVotes : 0
                        purchaseRequest.sponsorMaxRevotes = maxRevotes? maxRevotes : 0
                        purchaseRequest.sponsorIncentive = incentive? incentive * 10000 : 0
                        purchaseRequest.sponsorEndDate = endTime
                    }
                    var purchaseApi = purchaseRequest.submit()
                } catch (exception) {
                    NativeDialog.confirm(qsTr("Error creating contest"),
                                         qsTr("An error occurred when processing your request: %1").arg(exception),
                                         function(){}, false)
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
